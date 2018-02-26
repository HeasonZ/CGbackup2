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
vec4 cameraPos( 0, 0, -3.001,1 );
glm::mat4 R;
float yaw = 0;
/* ----------------------------------------------------------------------------*/

/* FUNCTIONS                                                                   */
void Update();
void Draw(screen* screen, const vector<Triangle>& triangles);
void VertexShader(const vec4& v, ivec2& p);
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL(screen* screen, ivec2 a, ivec2 b, vec3 color);
void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices);


int main( int argc, char* argv[] )
{
  vector<Triangle> triangles;
  LoadTestModel( triangles );
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  while( NoQuitMessageSDL() ){
    Update();
    Draw(screen,triangles);
    SDL_Renderframe(screen);
  }
  SDL_SaveImage( screen, "screenshot.bmp" );
  KillSDL(screen);
  return 0;

}



/*Place your drawing here*/

void Draw(screen* screen,const vector<Triangle>& triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  for( uint32_t i=0; i<triangles.size(); ++i ) {
    vector<vec4> vertices(3);
    vec3 color(1,1,1);
    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;
    DrawPolygonEdges(screen, vertices);
  }
}

/*Place updates of parameters here*/
void Update()
{

  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/

}

void VertexShader(const vec4& v, ivec2& p){
  vec4 pos = v-cameraPos;
  int f = SCREEN_HEIGHT; // focalLength
  p.x = (f*pos.x/pos.z) + (SCREEN_WIDTH/2);
  p.y = (f*pos.y/pos.z) + (SCREEN_HEIGHT/2);

}

void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result ){
  int N = result.size();
  vec2 step = vec2(b-a) / float(max(N-1,1));
  vec2 current(a);
  for( int i=0; i<N; ++i )
  {
    result[i] = current;
    current += step;
  }
}

void DrawLineSDL(screen* screen, ivec2 a, ivec2 b, vec3 color){
  ivec2 delta = glm::abs( a - b );
  int pixels = glm::max( delta.x, delta.y ) + 1;
  vector<ivec2> line( pixels );
  Interpolate( a, b, line );
  for(unsigned int i=0; i<line.size(); i++){
    vec3 color(1,1,1);
    PutPixelSDL(screen, line[i].x, line[i].y, color);
  }
}

void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices){
  int V = vertices.size();
  // Transform each vertex from 3D world position to 2D image position:
  vector<ivec2> projectedVertices( V );
  for( int i=0; i<V; ++i ){
    VertexShader( vertices[i], projectedVertices[i] );
  }
  // Loop over all vertices and draw the edge from it to the next vertex:
  for( int i=0; i<V; ++i ){
    int j = (i+1)%V; // The next vertex
    vec3 color( 1, 1, 1 );
    DrawLineSDL( screen, projectedVertices[i], projectedVertices[j], color);
  }
}
