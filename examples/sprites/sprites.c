/* Input Example - Astera 
   Last Updated: Feb 5 2020
   Last Author: tek256

   Feb 5 2020:
	- Created example
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <astera/render.h> // For the window management & rendering
#include <astera/sys.h> // For the time definitions
#include <astera/input.h> // For the keyboard, mouse, and controller usage

int8_t setup_render(){
  return 1;
}

void update(time_s delta){

}

void render(){

}

int main(int argc, char **argv) {
  if(!setup_render()){
     printf("Error: Unable to setup the rendering system.\n");
     return EXIT_FAILURE;
  }

  int8_t running = 1;

  while(running){
     update(delta);
     render();
  }

  return 0;
}
