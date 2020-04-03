/* Input Example - Astera 
   Last Updated: Feb 5 2020
   Last Author: tek256

   Feb 5 2020:
	- Created example
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <astera/sys.h> // For the time definitions
#include <astera/input.h> // For the keyboard, mouse, and controller usage
#include <astera/ui.h> // For the UI usage

int8_t create_ui(){
  return 1;
}

void update(time_s delta){

}

void render(){

}

int main(int argc, char **argv) {
  printf("Hello.\n");
	
  int8_t running = 0;

  while(running){
     update(delta);
     render();
  }

  return 0;
}
