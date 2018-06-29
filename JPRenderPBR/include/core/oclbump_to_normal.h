#pragma once
#include <amp.h>
#include <amp_math.h>

OCL_NAMESPACE_START

using namespace concurrency;
using namespace concurrency::fast_math;
index<2> SwapPosition(int x,int y,unsigned int width,unsigned int height,unsigned int wSwap,unsigned int hSwap)restrict(amp){
  index<2> res;
  int x0=0,y0=0;
  if(0<=x&&x<width)
    x0=x;
  else{
    switch(wSwap){
    case 0:
      x0=x%width;
      if(x0<0)x0+=width;
      break;
    case 1:
      if(x0<0)
        x0=-x;
      else x0=2*(width-1)-x;
      break;
    case 2:
      if(x0<0)x0=0;
      else x0=width-1;
      break;
    default:
      x0=-1;
    }
  }

  if(0<=y&&y<height)
    y0=y;
  else{
    switch(wSwap){
    case 0:
      y0=y%height;
      if(y0<0)y0+=height;
      break;
    case 1:
      if(y0<0)y0=-y0;
      else y0=2*(height-1)-y0;
      break;
    case 2:
      if(y0<0)y0=0;
      else y0=height-1;
      break;
    default:
      y0=-1;
    }
  }
  res[0]=y0;
  res[1]=x0;
  return res;
}
//swap:0,Ñ­»·;1,¾µÏñ;2,È¡±ßÔµ


void Bump2Normal(oclUint width,oclUint height,optix::float4* pInputData,optix::float4* pOutputData,oclUint wSwap,oclUint hSwap){

  array_view<optix::float4,2> in(height,width,pInputData);
  array_view<optix::float4,2> out(height,width,pOutputData);
  parallel_for_each(out.extent,[=](index<2> idx) restrict(amp){
    //float O = in[SwapPosition(idx[1], idx[0], width, height, wSwap, hSwap)].x;
    float h01=in[SwapPosition(idx[1]-1,idx[0],width,height,wSwap,hSwap)].x;
    float h21=in[SwapPosition(idx[1]+1,idx[0],width,height,wSwap,hSwap)].x;
    float h10=in[SwapPosition(idx[1],idx[0]+1,width,height,wSwap,hSwap)].x;
    float h12=in[SwapPosition(idx[1],idx[0]-1,width,height,wSwap,hSwap)].x;
    float h00=in[SwapPosition(idx[1]-1,idx[0]+1,width,height,wSwap,hSwap)].x;
    float h02=in[SwapPosition(idx[1]-1,idx[0]-1,width,height,wSwap,hSwap)].x;
    float h20=in[SwapPosition(idx[1]+1,idx[0]+1,width,height,wSwap,hSwap)].x;
    float h22=in[SwapPosition(idx[1]+1,idx[0]-1,width,height,wSwap,hSwap)].x;

    float Gx=(h00-h20+2.0f * h01-2.0f * h21+h02-h22);
    float Gy=(h00+2.0f * h10+h20-h02-2.0f * h12-h22);
    float Gz=1.f;

    float len=sqrt(4*Gx*Gx+4*Gy*Gy+Gz*Gz);
    out[idx].x=(Gx/len+1)/2;
    out[idx].y=(Gy/len+1)/2;
    out[idx].z=(Gz/len+1)/2;
    out[idx].w=0.f;
    //float left=in[SwapPosition(idx[1]-1,idx[0],width,height,wSwap,hSwap)].x;
    //float right=in[SwapPosition(idx[1]+1,idx[0],width,height,wSwap,hSwap)].x;
    //float top=in[SwapPosition(idx[1],idx[0]+1,width,height,wSwap,hSwap)].x;
    //float bottom=in[SwapPosition(idx[1],idx[0]-1,width,height,wSwap,hSwap)].x;

    //float r=0;
    //float g=0;
    //float b=0;

    //r=50*(left-right);
    //g=50*(top-bottom);
    //b=1.f;
    //float len=sqrt(r*r+g*g+b*b);
    //out[idx].x=(r/len+1)/2;
    //out[idx].y=(g/len+1)/2;
    //out[idx].z=(b/len+1)/2;
    //out[idx].w=0.f;
  });
}

OCL_NAMESPACE_END
