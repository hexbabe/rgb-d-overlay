#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 0

#include <iostream>
#include <memory>
#include <signal.h>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>

#include <viam/api/common/v1/common.grpc.pb.h>
#include <viam/api/component/generic/v1/generic.grpc.pb.h>
#include <viam/api/robot/v1/robot.pb.h>

#include <viam/sdk/components/component.hpp>
#include <viam/sdk/components/camera/camera.hpp>
#include <viam/sdk/config/resource.hpp>
#include <viam/sdk/module/module.hpp>
#include <viam/sdk/module/service.hpp>
#include <viam/sdk/registry/registry.hpp>
#include <viam/sdk/resource/resource.hpp>
#include <viam/sdk/rpc/dial.hpp>
#include <viam/sdk/rpc/server.hpp>

#include <Magick++.h>

std::tuple<uint64_t, uint64_t, std::vector<uint16_t>> deserialize_depth_map(const std::vector<unsigned char>& data) {
    if (data.size() < 24) {
        throw std::runtime_error("Data too short to contain valid depth information");
    }

    // Assuming data is in big-endian format
    uint64_t width = 0;
    uint64_t height = 0;

    // Extract width and height from the byte array header
    for (int i = 0; i < 8; ++i) {
        width = (width << 8) | data[8 + i];
        height = (height << 8) | data[16 + i];
    }

    if (data.size() < 24 + width * height * sizeof(uint16_t)) {
        throw std::runtime_error("Data size does not match width and height");
    }

    std::vector<uint16_t> arr;
    arr.reserve(width * height);

    // Remaining bytes are all depth map data
    for (size_t i = 0; i < width * height; ++i) {
        size_t data_index = 24 + i * sizeof(uint16_t);
        uint16_t depth_value = static_cast<uint16_t>(data[data_index] << 8 | data[data_index + 1]);
        arr.push_back(depth_value);
    }

    return std::make_tuple(width, height, arr);
}

using namespace viam::sdk;

class RGBDOverlay : public Camera {
   public:
    void reconfigure(Dependencies deps, ResourceConfig cfg) override {
        std::cout << "rgb-d-overlay " << Resource::name() << " is reconfiguring\n";
        if (deps.size() <= 0) {
            std::ostringstream buffer;
            buffer << "no dependencies were specified in config attributes\n";
            throw std::invalid_argument(buffer.str());
        } else if (deps.size() > 1) {
            std::ostringstream buffer;
            buffer << "please specify only one dependency\n";
            throw std::invalid_argument(buffer.str());
        }
        for (auto& dep : deps) {
            std::shared_ptr<Resource> cam_resource = dep.second;
            if (cam_resource->api().to_string() != API::get<Camera>().to_string()) {
                std::ostringstream buffer;
                buffer << "dependency " << cam_resource->api().to_string() << " is not a camera resource\n";
                throw std::invalid_argument(buffer.str());
            }
            cam = std::dynamic_pointer_cast<Camera>(cam_resource);
        }
    }

    RGBDOverlay(Dependencies deps, ResourceConfig cfg) : Camera(cfg.name()) {
        std::cout << "Creating rgb-d-overlay " + Resource::name() << std::endl;
        reconfigure(deps, cfg);
    }

    AttributeMap do_command(AttributeMap command) override {
        std::ostringstream buffer;
        buffer << "do_command method is unsupported\n";
        throw std::runtime_error(buffer.str());
    }

    std::vector<GeometryConfig> get_geometries(const AttributeMap& extra) override {
        return std::vector<GeometryConfig>();
    }

    raw_image get_image(std::string mime_type, const AttributeMap& extra) override {
        // Get image data from get_images call
        auto image_coll = cam->get_images();
        std::vector<raw_image> images = image_coll.images;
        std::string depth_format_str = Camera::format_to_MIME_string(viam::component::camera::v1::FORMAT_RAW_DEPTH);
        std::string jpeg_format_str = Camera::format_to_MIME_string(viam::component::camera::v1::FORMAT_JPEG);
        uint64_t width;
        uint64_t height;
        std::vector<uint16_t> depth_arr;
        std::vector<unsigned char> color_arr;
        for (raw_image image : images) {
            if (image.mime_type == depth_format_str) {
                std::tie(width, height, depth_arr) = deserialize_depth_map(image.bytes);
            } else if (image.mime_type == jpeg_format_str) {
                color_arr = image.bytes;
            } else {
                std::ostringstream buffer;
                buffer << "unsupported mimetype received from get_images: " << image.mime_type << std::endl;
                throw std::runtime_error(buffer.str());
            }
        }

        Magick::Image depth_image;
        depth_image.read(width, height, "I", Magick::ShortPixel, depth_arr.data());

        Magick::Blob color_blob(&color_arr[0], color_arr.size());
        Magick::Image color_image;
        color_image.read(color_blob);

        color_image.evaluate(Magick::AlphaChannel, Magick::MultiplyEvaluateOperator, 0.5);
        depth_image.evaluate(Magick::AlphaChannel, Magick::MultiplyEvaluateOperator, 0.5);

        // Soft light composite looks best visually
        color_image.composite(depth_image, 0, 0, Magick::SoftLightCompositeOp);

        Magick::Blob jpegBlob;
        color_image.magick("JPEG");
        color_image.write(&jpegBlob);

        const void* blobData = jpegBlob.data();
        std::vector<unsigned char> jpeg_bytes(static_cast<const unsigned char*>(blobData), static_cast<const unsigned char*>(blobData) + jpegBlob.length());

        raw_image overlayed_raw_image;
        overlayed_raw_image.mime_type = jpeg_format_str;
        overlayed_raw_image.bytes = jpeg_bytes;
        overlayed_raw_image.source_name = cam->name();
        return overlayed_raw_image;
    }

    image_collection get_images() override {
        std::ostringstream buffer;
        buffer << "get_images method is unsupported\n";
        throw std::runtime_error(buffer.str());
    }

    point_cloud get_point_cloud(std::string mime_type, const AttributeMap& extra) override {
        std::ostringstream buffer;
        buffer << "get_point_cloud method is unsupported\n";
        throw std::runtime_error(buffer.str());
    }

    properties get_properties() override {
        std::ostringstream buffer;
        buffer << "get_properties unsupported is unsupported\n";
        throw std::runtime_error(buffer.str());
    }

   private:
    std::shared_ptr<Camera> cam;
};

int main(int argc, char** argv) {
    Magick::InitializeMagick(nullptr);

    API camera_api = API::get<Camera>();
    Model m("viam", "camera", "rgb-d-overlay");

    std::shared_ptr<ModelRegistration> mr = std::make_shared<ModelRegistration>(
        camera_api,
        m,
        [](Dependencies deps, ResourceConfig cfg) { return std::make_unique<RGBDOverlay>(deps, cfg); },
        // Custom validation can be done by specifying a validate function like
        // this one. Validate functions can `throw` exceptions that will be
        // returned to the parent through gRPC. Validate functions can also return
        // a vector of strings representing the implicit dependencies of the resource.
        [](ResourceConfig cfg) -> std::vector<std::string> {
            return {};
        });

    std::vector<std::shared_ptr<ModelRegistration>> mrs = {mr};
    auto my_mod = std::make_shared<ModuleService>(argc, argv, mrs);
    my_mod->serve();

    return EXIT_SUCCESS;
};
