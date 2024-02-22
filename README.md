# Wave Deformer Plugin for Cinema 4D

![Wave Deformer Thumbnail](https://i.vimeocdn.com/video/470911455-bd36d409d211236488e8cbe0f6008483a27107d0eec7ba48cdff154a9a8918a4-d?mw=960&mh=540)

## About
Wave Deformer is an implementation of a Tessendorf wave for use in Maxon's Cinema 4D 3D software. It generates a single, keyframeable wave on a polygon-based object. (For optimal results, a subdivided plane should be utilized).

A brief demonstration of the plugin can be viewed in this [video](https://vimeo.com/johnny1k/wavedeformer).

Wave Deformer is derived from an original Python implementation by Vidar Nelson of Creative Tools (www.creativetools.se), released under the MIT license. For the original project, please visit https://github.com/CreativeTools/ct-ocean-waves. Vidar graciously granted permission for me to adapt his code to C++.

## Usage
Notably faster, particularly with high-poly geometry, the C++ Wave Deformer includes additional advanced features accessible through the Preview tab. When 'Use Length Falloff' is enabled, the gradient below defines the intensity of the effect along the wave, allowing for a smooth 'fade out' at the edges. For an even more seamless integration, an additional mask can be enabled with the 'Use Mask' checkbox. The shader assigned in the mask slot will be projected from the top onto the wave, mirroring the intensity control of the 'Length Falloff' setting.

## Notes
The initial Wave Deformer was released as a closed-source plugin in 2014 and tested in Cinema 4D R14 and R15 on PC (32 and 64-bit), as well as R15 on Mac. To the best of my knowledge, subsequent versions also functioned, but due to an architectural change in Cinema 4D R20, a new compilation was required, which unfortunately I never completed.

I am opening the source code of Wave Deformer so that individuals with stronger C++ programming skills can take on the project and breathe new life into it. A fresh compile for current versions would be a solid starting point, although there are undoubtedly numerous other enhancements that could be incorporated into my admittedly subpar code.
