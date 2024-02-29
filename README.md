# RGB Depth Overlay modular component

This module implements the [Viam camera API](https://docs.viam.com/build/program/apis/#camera) in a `rgb-d-overlay` model.
With this model, you can process color and depth outputs from multiple camera modules, in order to ensure that the FOV, resolution, aspect ratio, and time synchronization of the cameras are aligned.
This module uses ImageMagick to overlay two images semi-transparently to directly compare two camera outputs.

## Requirements

Your machine must have at least one [camera component](https://docs.viam.com/components/camera/) which supports outputting simultaneous depth and color image streams, such as the [Intel Realsense](https://app.viam.com/module/viam/realsense) or the [Luxonis OAK-D](https://app.viam.com/module/viam/oak-d), in order to use the `rgb-d-overlay` module.

## Build and run

To use this module, follow the instructions to [add a module from the Viam Registry](https://docs.viam.com/registry/configure/#add-a-modular-resource-from-the-viam-registry) and select the `viam:camera:rgb-d-overlay` model from the [`rgb-d-overlay` module](https://app.viam.com/module/viam/rgb-d-overlay).

Alternatively, you can [build this module yourself](#build-the-rgb-d-overlay-module), and add it to your machine as a local module.

## Configure your `rgb-d-overlay` camera component

> [!NOTE]
> Before configuring your camera component, you must [create a machine](https://docs.viam.com/manage/fleet/machines/#add-a-new-machine).

Navigate to the **Config** tab of your machine's page in [the Viam app](https://app.viam.com/).
Click on the **Components** subtab and click **Create component**.
Select the `camera` type, then select the `rgb-d-overlay` model.
Click **Add module**, then enter a name for your camera component and click **Create**.

> [!NOTE]
> For more information, see [Configure a Machine](https://docs.viam.com/manage/configuration/).

### Configure your image stream camera

On the camera component that supports outputting simultaneous depth and color image streams, such as the [Intel Realsense](https://app.viam.com/module/viam/realsense) or the [Luxonis OAK-D](https://app.viam.com/module/viam/oak-d), configure the `sensors` attribute to support both types of camera streams supported:

Navigate to the **Config** tab of your machine's page in [the Viam app](https://app.viam.com/).
Click on the **Components** subtab and select the configuration pane for your image stream camera, _not_ the `rgb-d-overlay` camera.
Under **Attributes**, add the following `sensors` configuration to your image stream camera configuration:

```json
{
    "sensors": ["color", "depth"]
}
```

Click **Save config** to save your changes.

## Attributes

There are no attributes available for configuration with this component.

## Example configuration

```json
{
  // ... existing configuration ...
  "components": [
    // ... other components ...
    {
      "name": "my-rgb-d-overlay",
      "model": "viam:camera:rgb-d-overlay",
      "type": "camera",
      "namespace": "rdk",
      "depends_on": ["<name-of-camera-to-get-images-from>"]
    }
  ]
}
```

Update the `depends_on` array with the name of your image stream camera.
For example, if your image stream camera is named `my-realsense-cam`, you would use the following configuration: `"depends_on": ["my-realsense-cam"]`

## Next steps

Once you have added the `rgb-d-overlay` module, you can use it to compare camera outputs from the Viam app.

1. Navigate to the **Control** tab for your machine in the [Viam app](https://app.viam.com/), and select the `rgb-d-overlay` camera
1. Enable the camera stream to see the live camera feed, including the overlay.

## Build the `rgb-d-overlay` module

You can also build this module yourself using the instructions below:

### Build on MacOS

1. Install the required dependencies by running the `setup.sh` script from this repository.
1. [Install Rust](https://www.rust-lang.org/tools/install).
1. Clone this repository:

   ```bash
   git clone https://github.com/viam-labs/rgb-d-overlay.git
   cd rgb-d-overlay
   ```

1. Build the module:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

The outputted binary `rgb-d-overlay` will be written to the `build` directory.

### Build on Linux

1. [Install Docker](https://docs.docker.com/engine/install/).
1. Clone this repository:

   ```bash
   git clone https://github.com/viam-labs/rgb-d-overlay.git
   cd rgb-d-overlay
   ```

1. Run `make appimage-aarch64`

### Add as a local module

Once you have built the `rgb-d-overlay` module, you can [add it to your machine as a local module](https://docs.viam.com/registry/configure/#add-a-local-module).
