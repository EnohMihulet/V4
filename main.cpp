#include <iostream>
#include <string>
#include <vector>

#include "chess/Common.h"
#include "movegen/MoveGenTest.h"
#include "search/EvaluationTests.h"
#include "helpers/GameStateHelper.h"
#include "search/Search.h"

int main() {
	
	GameState gameState((std::string) DEFAULT_FEN_POSITION);
	std::vector<MoveInfo> history;
	history.reserve(64);
	Move move = iterativeDeepeningSearch(gameState, history);
	std::cout << move.moveToString() << std::endl;

	std::string command;
	while (std::getline(std::cin, command)) {
		if (command == "uci") {
			std::cout << "id name ChessV4" << std::endl;
			std::cout << "id author EnohMihulet" << std::endl;
			std::cout << "uciok" << std::endl;
		}
		else if (command == "isready") {
			std::cout << "readyok" << std::endl;
		}
		else if (command.rfind("position", 0) == 0) {
			
		}
		else if (command.rfind("go", 0) == 0) {
			
			std::cout << "bestmove e2e4" << std::endl;
		}
		else if (command == "quit") {
			break;
		}
	}

	return 0;
}
