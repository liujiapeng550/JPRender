# JPRender
JPRender是由liujiapeng独立开发的基于物理的渲染。它使用c++构建，利用Nvida Cuda和optix GPU多线程，提高渲染效率，和质量。它包括射线追踪模拟，材质，采样，AI Denoiser,相机模型，assimp模型加载模块

包涵一下库
CUDA
Optix
glew
glfw;
assimp
boost
在visualStudio 2015开发
## Screenshots
### 开启AI Denoiser的车漆材质
![](https://github.com/liujiapeng550/JPRender/blob/master/img/carpaint.bmp?raw=true)
### 关闭AI Denoiser的车漆材质 边缘会有明显的噪点
![](https://github.com/liujiapeng550/JPRender/blob/master/img/carpaintNoDe.bmp?raw=true)
### 皮革材质 用的法线贴图
![](https://github.com/liujiapeng550/JPRender/blob/master/img/leather.bmp?raw=true)
### meta材质,开启全局光照
![](https://github.com/liujiapeng550/JPRender/blob/master/img/meta.png?raw=true)
### 金属材质
![](https://github.com/liujiapeng550/JPRender/blob/master/img/metal.png?raw=true)
### 金属拉丝材质
![](https://github.com/liujiapeng550/JPRender/blob/master/img/metalBrushed.bmp?raw=true)
### 橡胶材质
![](https://github.com/liujiapeng550/JPRender/blob/master/img/rubber.bmp?raw=true)
### 木纹材质
![](https://github.com/liujiapeng550/JPRender/blob/master/img/wood.bmp?raw=true)
### 玻璃材质
![](https://github.com/liujiapeng550/JPRender/blob/master/img/glass.bmp?raw=true)
