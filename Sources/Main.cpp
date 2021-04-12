#include "Simple2D.h"
#include "Game/Game.h"

int main(int argc, char *argv[])
{
	Simple2D::Window* pWindow = Simple2D::CreateWindow("ShooterGame", Game::mWindowW, Game::mWindowH);

	Game::GameInit();

	while (!Simple2D::ShouldWindowClose(pWindow))
	{
		Game::GameStep();

		Simple2D::RefreshWindowBuffer(pWindow);
	}

	Simple2D::DestroyWindow(pWindow);
}

