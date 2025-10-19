#include <iostream>
#include <vector>

#include <SDL.h>

#include "MoveGen.h"
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_sdl2.h"
#include "external/imgui/backends/imgui_impl_sdlrenderer2.h"

#include "Common.h"
#include "GameState.h"
#include "gui/GuiCommon.h"
#include "gui/BoardView.h"


int main() {
	
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init failed." << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Chess GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (!window) {
		std::cerr << "SDL_CreateWindow failed." << std::endl;
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer failed." << std::endl;
		return 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	// GameState gameState((std::string) DEFAULT_FEN_POSITION);
	// GameState gameState("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
	GameState gameState("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
	GuiState guiState;
	guiState.history.reserve(64);
	guiState.movesMade.reserve(64);
	guiState.allMoves.reserve(256);
	guiState.selectedPieceMoves.reserve(24);
	
	generateAllMoves(gameState, guiState.allMoves, gameState.colorToMove, guiState.checkMask, guiState.pinnedPieces, guiState.pinnedRays);

	bool running = true;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
		}

		ImGui_ImplSDL2_NewFrame();
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui::NewFrame();
		
		drawFenString(gameState);

		drawBoard(gameState, guiState);
		
		drawBitboards(gameState, guiState);

		drawToggles(guiState);

		drawUndoButton(gameState, guiState);

		drawSelectPinnedRayIndex(guiState);

		drawPinnedRayBitboard(guiState);
		
		drawIsSquareAttacked(gameState);

		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
	}

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

