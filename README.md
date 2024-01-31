# RGB Depth Overlay C++ Module

The RGB-D overlay module is designed to process color and depth outputs from a camera module, calling `get_images`. It uses ImageMagick to overlay the two images semi-transparently to directly compare the outputs and check if they are aligned.

### Setup

1. Install necessary packages from [setup.sh](setup.sh).
1. Install Rust from [here](https://www.rust-lang.org/tools/install) for `viam_rust_utils`.

## Building and Running the Module
1. Clone this repository
```
git clone https://github.com/viam-labs/rgb-d-overlay.git
cd rgb-d-overlay
```
1. Run
```
mkdir build
cd build
cmake ..
make
```
1. The outputted binary `rgb-d-overlay` should be in the `build` directory.

### Local Build

Follow steps for [preparing a module for execution](https://docs.viam.com/registry/create/#prepare-the-module-for-execution). Use sections linked in [Module Contents](#module-contents) as a reference.

## Module Configuration

After [adding a machine](https://docs.viam.com/fleet/machines/#add-a-new-machine), navigate to **Config** -> **Components** in the Viam app and create a component using the `rgb-d-overlay` model. Ensure your JSON configuration under **Raw JSON** mode includes the new component and module.

Example configuration snippet:

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
