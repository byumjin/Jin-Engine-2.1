#include "Render\Renderer.h"

int main()
{
	Renderer renderer;

	renderer.initialize(NULL);


	renderer.mainloop();


	renderer.shutDown();


	return EXIT_SUCCESS;
}