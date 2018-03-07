#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>



using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::ivec2;
using glm::vec2;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false
glm::mat3 R;
float yaw = 0;
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

struct Pixel{
  int x;
  int y;
  float zinv;
};

/* ----------------------------------------------------------------------------*/

/* FUNCTIONS                                                                   */
void Update(vec4 &cameraPos);
void Draw(vec4 cameraPos, screen* screen, const vector<Triangle>& triangles);
void VertexShader(vec4 cameraPos, const vec4& v, ivec2& p);
void VertexShaderPixel(vec4 cameraPos, const vec4& v, Pixel& p);

void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void InterpolatePixel( Pixel a, Pixel b, vector<Pixel>& result );

void DrawLineSDL(screen* screen, ivec2 a, ivec2 b, vec3 color);
void DrawLineSDLPixel(screen* screen, Pixel a, Pixel b, vec3 color);

void DrawPolygonEdges(vec4 cameraPos,screen* screen, const vector<vec4>& vertices);

void ComputePolygonRows(const vector<ivec2>& vertexPixels,vector<ivec2>& leftPixels,vector<ivec2>& rightPixels);
void ComputePolygonRowsPixel(const vector<Pixel>& vertexPixels,vector<Pixel>& leftPixels,vector<Pixel>& rightPixels);

void DrawRows(screen* screen, const vector<ivec2>& leftPixels,const vector<ivec2>& rightPixels, vec3 color);
void DrawRowsPixel(screen* screen, const vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels, vec3 color);

void DrawPolygon(vec4 cameraPos, screen* screen, const vector<vec4>& vertices, vec3 color);
void DrawPolygonPixel(vec4 cameraPos, screen* screen, const vector<vec4>& vertices, vec3 color);


int main( int argc, char* argv[] ){
  vector<Triangle> triangles;
  LoadTestModel( triangles );
  vec4 cameraPos(0, 0, -3.001,1);
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  //Test
  vector<Pixel> vertexPixels(3);
  // Pixel a = {10, 5, 10};
  vertexPixels[0] = Pixel{10, 5, 16};
  vertexPixels[1] = Pixel{5, 10, 15 };
  vertexPixels[2] = Pixel{15,15, 15};
  vector<Pixel> leftPixels;
  vector<Pixel> rightPixels;
  ComputePolygonRowsPixel( vertexPixels, leftPixels, rightPixels);
  for( int row=0; row<leftPixels.size(); ++row )
  {
    cout << "Start: ("
    << leftPixels[row].x << ","
    << leftPixels[row].y << ","
    << leftPixels[row].zinv << "). "
    << "End: ("
    << rightPixels[row].x << ","
    << rightPixels[row].y << ","
    << rightPixels[row].zinv <<"). " << endl;
  }

  while( NoQuitMessageSDL() ){
    Update(cameraPos);
    Draw(cameraPos,screen,triangles);
    SDL_Renderframe(screen);
  }
  SDL_SaveImage( screen, "screenshot.bmp" );
  KillSDL(screen);
  return 0;

}

/*Place your drawing here*/
void Draw(vec4 cameraPos, screen* screen,const vector<Triangle>& triangles){
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  for( int y=0; y<SCREEN_HEIGHT; ++y){
    for( int x=0; x<SCREEN_WIDTH; ++x){
          depthBuffer[y][x] = 0;
    }
  }
  for( uint32_t i=0; i<triangles.size(); ++i ) {
    vector<vec4> vertices(3);

    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;
    vec3 color  = triangles[i].color;
    DrawPolygonPixel(cameraPos, screen, vertices, color);
    // DrawPolygon(cameraPos, screen, vertices, color);
    // DrawPolygonEdges(cameraPos,screen, vertices);
  }
}

/*Place updates of parameters here*/
void Update(vec4 &cameraPos){

  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
  const uint8_t* keystate = SDL_GetKeyboardState( 0 );
  if( keystate[SDL_SCANCODE_UP] ){
    cameraPos[2] = cameraPos[2]+0.01;
  }

  if( keystate[SDL_SCANCODE_DOWN]){
    cameraPos[2] = cameraPos[2]-0.01;
  }

  if(keystate[SDL_SCANCODE_LEFT]){
    yaw = yaw - 0.1;
    //Rotation
    R = mat3(cos(yaw), 0.0f, sin(yaw), 0.0f, 1.0f, 0.0f, -sin(yaw), 0.0f,
                  cos(yaw));
  }
  if( keystate[SDL_SCANCODE_RIGHT] ){
    yaw = yaw + 0.1;
    R = mat3(cos(yaw), 0.0f, sin(yaw), 0.0f, 1.0f, 0.0f, -sin(yaw), 0.0f,
                  cos(yaw));
  }
}

// void VertexShader(vec4 cameraPos, const vec4& v, ivec2& p){
//   vec4 pos = (v-cameraPos); // transform
//   int f = SCREEN_HEIGHT;    // focalLength
//   p.x = (f*pos.x/pos.z) + (SCREEN_WIDTH/2);
//   p.y = (f*pos.y/pos.z) + (SCREEN_HEIGHT/2);
// }

void VertexShaderPixel(vec4 cameraPos, const vec4& v, Pixel& p){
  vec4 pos = (v-cameraPos); // transform
  int f = SCREEN_HEIGHT;    // focalLength
  p.x = (f*pos.x/pos.z) + (SCREEN_WIDTH/2);
  p.y = (f*pos.y/pos.z) + (SCREEN_HEIGHT/2);
  p.zinv = 1/pos.z;
  std::cout << "/* message */" << p.zinv <<'\n';
}

// void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result){
//   int N = result.size();
//   vec2 step = vec2(b-a) / float(max(N-1,1));
//   vec2 current(a);
//   for( int i=0; i<N; ++i )
//   {
//     result[i] = current;
//     current += step;
//   }
// }

void InterpolatePixel( Pixel a, Pixel b, vector<Pixel>& result ){
  int N = result.size();
  float stepX = (b.x-a.x) / float(max(N-1,1));
  float stepY = (b.y-a.y) / float(max(N-1,1));
  float stepZ = (b.zinv-a.zinv) / float(max(N-1,1));
  float currentX = a.x, currentY = a.y, currentZ = a.zinv;
  // cannot do initialize in that way.
  // Pixel current(a);

  for( int i=0; i<N; ++i ){
    result[i].x = (int) currentX;
		result[i].y = (int) currentY;
    result[i].zinv = currentZ;
    // result[i] = current;
    // current.x += stepX;
    // current.y += stepY;
    // current.zinv += stepZ;
    currentX += stepX;
		currentY += stepY;
    currentZ += stepZ;
  }

}

// void DrawLineSDL(screen* screen, ivec2 a, ivec2 b, vec3 color){
//   ivec2 delta = glm::abs( a - b );
//   int pixels = glm::max( delta.x, delta.y ) + 1;
//   vector<ivec2> line( pixels );
//   Interpolate(a, b, line);
//   for(unsigned int i=0; i<line.size(); i++){
//     PutPixelSDL(screen, line[i].x, line[i].y, color);
//   }
// }

void DrawLineSDLPixel(screen* screen, Pixel a, Pixel b, vec3 color){
  ivec2 a_tmp(a.x, a.y);
  ivec2 b_tmp(b.x, b.y);
  ivec2 delta = glm::abs(a_tmp - b_tmp);
  int pixels = glm::max( delta.x, delta.y ) + 1;
  vector<Pixel> line(pixels);
  InterpolatePixel(a, b, line);
  for(unsigned int i=0; i<line.size(); i++){
    // if(line[i].x>=0 && line[i].x<SCREEN_WIDTH && line[i].y>=0 && line[i].y<SCREEN_HEIGHT){
      if(line[i].zinv >= depthBuffer[line[i].y][line[i].x]){
        depthBuffer[line[i].y][line[i].x] = line[i].zinv;
        PutPixelSDL(screen, line[i].x, line[i].y, color);
      }
    // }
  }
}

// void DrawPolygonEdges(vec4 cameraPos,screen* screen, const vector<vec4>& vertices){
//   int V = vertices.size();
//   // Transform each vertex from 3D world position to 2D image position:
//   vector<ivec2> projectedVertices( V );
//   for( int i=0; i<V; ++i ){
//     VertexShader(cameraPos,vertices[i], projectedVertices[i] );
//   }
//   // Loop over all vertices and draw the edge from it to the next vertex:
//   for( int i=0; i<V; ++i ){
//     int j = (i+1)%V; // The next vertex
//     vec3 color( 1, 1, 1 );
//     DrawLineSDL( screen, projectedVertices[i], projectedVertices[j], color);
//   }
// }

void TransformationMatrix(glm::mat4x4 M){
}

// void ComputePolygonRows(const vector<ivec2> &vertexPixels, vector<ivec2>& leftPixels,vector<ivec2>& rightPixels){
//   // 1. Find max and min y-value of the polygon
//   //and compute the number of rows it occupies.
//   int miny = numeric_limits<int>::max();  // 1000
//   int maxy = -numeric_limits<int>::max(); //-1000
//   for (unsigned int i=0; i<vertexPixels.size();i++){
//     if (vertexPixels[i].y < miny){
//       miny = vertexPixels[i].y;
//     }
//     if (vertexPixels[i].y > maxy){
//       maxy = vertexPixels[i].y;
//     }
//   }
//   int numRows = maxy - miny + 1;
//   // 2. Resize leftPixels and rightPixels
//   // so that they have an element for each row.
//   leftPixels.resize(numRows);
//   rightPixels.resize(numRows);
//
//   // 3. Initialize the x-coordinates in leftPixels
//   // to some really large value and the x-coordinates
//   // in rightPixels to some really small value.
//   for (unsigned int i=0; i<leftPixels.size(); i++){
//     leftPixels[i].x  = +numeric_limits<int>::max();
//     rightPixels[i].x = -numeric_limits<int>::max();
//   }
//
//   // 4. Loop through all edges of the polygon and use
//   // linear interpolation to find the x-coordinate for
//   // each row it occupies. Update the corresponding
//   // values in rightPixels and leftPixels.
//   int V = vertexPixels.size();
//   for(int i=0; i<V; ++i){
//     int j = (i+1)%V;
//     int top = glm::min(vertexPixels[i].y, vertexPixels[j].y);
//     int bot = glm::max(vertexPixels[i].y, vertexPixels[j].y);
//     int pixels = bot - top + 1;
//
//     vector<ivec2> line( pixels );
//     Interpolate(vertexPixels[i], vertexPixels[j], line);
//     for (unsigned int j=0; j<line.size(); j++){
//       if(line[j].x < leftPixels[line[j].y-miny].x){
//         leftPixels[line[j].y-miny].x = line[j].x;
//         leftPixels[line[j].y-miny].y = line[j].y;
//
//       }
//       if(line[j].x > rightPixels[line[j].y-miny].x){
//         rightPixels[line[j].y-miny].x = line[j].x;
//         rightPixels[line[j].y-miny].y = line[j].y;
//
//       }
//     }
//   }
//
// }

void ComputePolygonRowsPixel(const vector<Pixel>& vertexPixels,vector<Pixel>& leftPixels,vector<Pixel>& rightPixels){
  // 1. Find max and min y-value of the polygon
  //and compute the number of rows it occupies.
  int miny = numeric_limits<int>::max();  // 1000
  int maxy = -numeric_limits<int>::max(); //-1000
  for (unsigned int i=0; i<vertexPixels.size();i++){
    if (vertexPixels[i].y < miny){
      miny = vertexPixels[i].y;
    }
    if (vertexPixels[i].y > maxy){
      maxy = vertexPixels[i].y;
    }
  }
  int numRows = maxy - miny + 1;
  // 2. Resize leftPixels and rightPixels
  // so that they have an element for each row.
  leftPixels.resize(numRows);
  rightPixels.resize(numRows);
  // 3. Initialize the x-coordinates in leftPixels
  // to some really large value and the x-coordinates
  // in rightPixels to some really small value.
  for (unsigned int i=0; i<leftPixels.size(); i++){
    leftPixels[i].x  = +numeric_limits<int>::max();
    rightPixels[i].x = -numeric_limits<int>::max();
  }
  // 4. Loop through all edges of the polygon and use
  // linear interpolation to find the x-coordinate for
  // each row it occupies. Update the corresponding
  // values in rightPixels and leftPixels.
  int V = vertexPixels.size();
  for(int i=0; i<V; ++i){
    int j = (i+1)%V;
    int top = glm::min(vertexPixels[i].y, vertexPixels[j].y);
    int bot = glm::max(vertexPixels[i].y, vertexPixels[j].y);
    int pixels = bot - top + 1;

    vector<Pixel> line(pixels);
    InterpolatePixel(vertexPixels[i], vertexPixels[j], line);
    for (unsigned int j=0; j<line.size(); j++){
      if(line[j].x < leftPixels[line[j].y-miny].x){
        leftPixels[line[j].y-miny].x = line[j].x;
        leftPixels[line[j].y-miny].y = line[j].y;
        leftPixels[line[j].y-miny].zinv = line[j].zinv;
        // std::cout << "/* message1 */" << line[j].zinv <<'\n';
      }
      if(line[j].x > rightPixels[line[j].y-miny].x){
        rightPixels[line[j].y-miny].x = line[j].x;
        rightPixels[line[j].y-miny].y = line[j].y;
        rightPixels[line[j].y-miny].zinv = line[j].zinv;
        // std::cout << "/* message2 */" << line[j].zinv <<'\n';
      }
    }
  }
}

// void DrawRows(screen* screen, const vector<ivec2>& leftPixels,const vector<ivec2>& rightPixels, vec3 color){
//   for(unsigned int i=0; i<leftPixels.size(); i++){
//     DrawLineSDL(screen, leftPixels[i], rightPixels[i], color);
//   }
// }

void DrawRowsPixel(screen* screen, const vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels, vec3 color){
  for(unsigned int i=0; i<leftPixels.size(); i++){
    int numCols = rightPixels[i].x - leftPixels[i].x + 1;
    vector<Pixel> line(numCols);
    InterpolatePixel(rightPixels[i], leftPixels[i], line);
    for(unsigned int j=0; j<line.size(); j++){
      if(line[j].zinv >= depthBuffer[line[j].y][line[j].x]){
        PutPixelSDL(screen, line[j].x, line[j].y, color);
        depthBuffer[line[j].y][line[j].x] = line[j].zinv;
      }
    }
    // DrawLineSDLPixel(screen, leftPixels[i], rightPixels[i], color);
  }
}

// void DrawPolygon(vec4 cameraPos, screen* screen, const vector<vec4>& vertices, vec3 color){
//   int V = vertices.size();
//   vector<ivec2> vertexPixels(V);
//   for(int i=0; i<V; ++i){
//     VertexShader(cameraPos, vertices[i], vertexPixels[i]);
//   }
//   vector<ivec2> leftPixels;
//   vector<ivec2> rightPixels;
//   ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
//   DrawRows(screen, leftPixels, rightPixels, color);
// }

void DrawPolygonPixel(vec4 cameraPos, screen* screen, const vector<vec4>& vertices, vec3 color){
  int V = vertices.size();
  vector<Pixel> vertexPixels(V);
  for(int i=0; i<V; ++i){
    VertexShaderPixel(cameraPos, vertices[i], vertexPixels[i]);
  }
  vector<Pixel> leftPixels;
  vector<Pixel> rightPixels;
  ComputePolygonRowsPixel(vertexPixels, leftPixels, rightPixels);
  DrawRowsPixel(screen, leftPixels, rightPixels, color);
}
