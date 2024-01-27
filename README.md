This is a practice project of using DirectX 12 API and its features.

The renderer can parse fbx files and render fbx objects in a scene. This renderer is using a GPU driven pipeline. All of the culling and the drawing are done by GPU. CPU is only resposible for preparing data.

1. The Frustum Culling pass uses downsampled ray tracing to cull objects outside of the view of camera.
![](/Readme/1.PNG)

2. The Indrect drawing pass only appends unculled commands to the argument buffer.

3. The GBuffer executes drawing commands in the argumment buffer, and pass stores the information for shading objects on multiple RTVs.
![](/Readme/2.PNG)

4. The Deferred Lighting uses the information from Gbuffers to shade objects.
![](/Readme/3.PNG)

5. The Raytracing pass renders features of GI, AO, light shadow and skybox.
![](/Readme/4.PNG)

6. The TAA pass denoise the result, and remove the aliasing.
![](/Readme/5.PNG)
