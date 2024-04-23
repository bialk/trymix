

#### to rename files in the directory by pattern:

```
find ShaderMix -type f -name "*" -exec sh -c 'mv "$1" "${1/CameraControl_/ShaderMix_}"' _ {} \;
```
