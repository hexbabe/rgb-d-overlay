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

std::vector<std::vector<int>> bytes_to_depth_array(const std::vector<uint8_t>& data) {
    if (data.size() < 24) {
        throw std::runtime_error("Data too short to contain valid depth information");
    }

    // Assuming data is in big-endian format
    uint64_t width = 0;
    uint64_t height = 0;

    // Extract width and height from the byte array
    for (int i = 0; i < 8; ++i) {
        width = (width << 8) | data[8 + i];
        height = (height << 8) | data[16 + i];
    }

    if (data.size() < 24 + width * height * sizeof(uint16_t)) {
        throw std::runtime_error("Data size does not match width and height");
    }

    std::vector<std::vector<int>> depth_arr_2d(height, std::vector<int>(width));

    // Convert the remaining bytes to a 2D depth array
    for (uint64_t row = 0; row < height; ++row) {
        for (uint64_t col = 0; col < width; ++col) {
            size_t data_idx = 24 + (row * width + col) * sizeof(uint16_t);
            uint16_t depth_val = static_cast<uint16_t>(data[data_idx] << 8 | data[data_idx + 1]);
            depth_arr_2d[row][col] = depth_val;
        }
    }

    return depth_arr_2d;
}

using namespace viam::sdk;

class AlignmentChecker : public Camera {
   public:
    void reconfigure(Dependencies deps, ResourceConfig cfg) override {
        std::cout << "AlignmentChecker " << Resource::name() << " is reconfiguring\n";
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

    AlignmentChecker(Dependencies deps, ResourceConfig cfg) : Camera(cfg.name()) {
        std::cout << "Creating AlignmentChecker " + Resource::name() << std::endl;
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
        auto image_coll = cam->get_images();
        auto duration_since_epoch = image_coll.metadata.captured_at.time_since_epoch();
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration_since_epoch).count();
        std::cout << "captured at: " << nanoseconds << std::endl;

        std::vector<raw_image> images = image_coll.images;
        raw_image return_img;
        for (raw_image img : images) {
            std::string depth_format_str = Camera::format_to_MIME_string(viam::component::camera::v1::FORMAT_RAW_DEPTH);
            std::string jpeg_format_str = Camera::format_to_MIME_string(viam::component::camera::v1::FORMAT_JPEG);
            if (img.mime_type == depth_format_str) {
                std::cout << img.mime_type << " depth\n";
            } else if (img.mime_type == jpeg_format_str) {
                return_img = img;
                std::cout << img.mime_type << " jpeg\n";
            } else {
                std::ostringstream buffer;
                buffer << "unsupported image mimetype received from get_images call to camera: " << img.mime_type;
                throw std::runtime_error(buffer.str());
            }
        }
        return return_img;
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
    API camera_api = API::get<Camera>();
    Model m("viam", "camera", "alignment-checker");

    std::shared_ptr<ModelRegistration> mr = std::make_shared<ModelRegistration>(
        camera_api,
        m,
        [](Dependencies deps, ResourceConfig cfg) { return std::make_unique<AlignmentChecker>(deps, cfg); },
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
