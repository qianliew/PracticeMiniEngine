This is a practice project of using DirectX 12 API and its features.

The renderer can parse fbx files and render it in a scene.

The GBuffer pass stores the information for shading objects on multiple RTVs.
![](/Readme/2.PNG)

The Deferred Lighting uses the information from Gbuffers to shade objects.
![](/Readme/3.PNG)

The Raytracing pass renders features of GI, AO, light shadow and skybox.
![](/Readme/4.PNG)

The TAA pass denoise the result, and remove the aliasing.
![](/Readme/5.PNG)
