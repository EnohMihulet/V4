#include <vector>


#include "../external/imgui/imgui.h"

#include "BoardView.h"

#include "GuiCommon.h"
#include "../chess/GameState.h"
#include "../movegen/MoveGen.h"


void drawFenString(GameState &gameState) {
	std::string fen = gameState.toFenString();

	ImGui::Begin("FEN String");
	ImGui::Text("%s", fen.c_str());
	ImGui::End();
}

void drawBoard(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Board");

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 windowPos = ImGui::GetWindowPos();

	float squareSize = 50.0f;
	int16 numCols = 8;
	int16 numRows = 8;
	int16 boardHeight = numRows * squareSize;
	
	ImVec2 headerMin = ImVec2(windowPos.x, windowPos.y);
	ImVec2 headerMax = ImVec2(windowPos.x + squareSize * numCols, windowPos.y + 20);
	drawList->AddRectFilled(headerMin, headerMax, BLACK_COLOR);

	for (int16 row = 0; row < numRows; row++) {
		for (int16 col = 0; col < numCols; col++) {
			ImVec2 sqMin = ImVec2(windowPos.x + col * squareSize, headerMax.y + (numRows - 1 - row) * squareSize);
			ImVec2 sqMax = ImVec2(sqMin.x + squareSize, sqMin.y + squareSize);
			uint8 index = (row * numCols + col);

			ImGui::SetCursorScreenPos(sqMin);
			ImGui::PushID(index);
			bool pressed = ImGui::InvisibleButton("tile", ImVec2(squareSize, squareSize));
			ImGui::PopID();

			if (pressed) {
				Piece pieceAtSq = gameState.pieceAt(index);
				if (pieceAtSq != EMPTY && getPieceColor(pieceAtSq) == gameState.colorToMove && guiState.selectedPieceSq != index) {
					guiState.selectedPieceSq = index;
					guiState.selectedPieceMoves.clear();
					for (Move move : guiState.allMoves) {
						if (move.getStartSquare() == guiState.selectedPieceSq) guiState.selectedPieceMoves.push_back(move);
					}
				}
				else {
					for (Move& move : guiState.selectedPieceMoves) {
						if (index == move.getTargetSquare() && guiState.selectedPieceSq == move.getStartSquare()) {
							guiState.selectedPieceMoves.clear();
							guiState.selectedPieceSq = -1;

							gameState.makeMove(move, guiState.history);
							guiState.movesMade.push_back(move);

							generateAllMoves(gameState, guiState.allMoves, gameState.colorToMove, guiState.checkMask, guiState.pinnedPieces, guiState.pinnedRays);
						}
					}
				}
			}

			ImU32 color = ((row + col) % 2 == 0) ? DARK_SQUARE_COLOR : LIGHT_SQUARE_COLOR ;
			for (Move& move : guiState.selectedPieceMoves) {
				if (guiState.selectedPieceSq == -1) break;
				if (move.getTargetSquare() == index) color = MOVE_HIGHLIGHT_COLOR;
			}
			if (guiState.showCheckMask) {
				if (1ULL << index & guiState.checkMask) color = CHECK_MASK_COLOR;
			}
			if (guiState.showPinnedPieces) {
				if (1ULL << index & guiState.pinnedPieces) color = PINNED_PIECES_COLOR;
			}
			if (guiState.showPinnedRays) {
				if (guiState.pinnedRays[index] != 0 && (guiState.pinnedRays[index] & 1ULL << index)) color = PINNED_RAYS_COLOR;
			}


			drawList->AddRectFilled(sqMin, sqMax, color);


			const char ch = pieceToChar(gameState.board[index]);
			if (ch != '\0') {
				ImFont* font = ImGui::GetFont();
				float base = ImGui::GetFontSize();
				float scale = 1.6f;
				float fsize = base * scale;
			
				bool is_white_piece = (ch >= 'A' && ch <= 'Z');
				ImU32 piece_color = is_white_piece ? WHITE_COLOR : BLACK_COLOR;
			
				char pieceChar[2] = { ch, '\0' };
			
				ImVec2 textSize = font->CalcTextSizeA(fsize, FLT_MAX, 0.0f, pieceChar);
				ImVec2 text_pos = ImVec2(sqMin.x + 0.5f * (squareSize - textSize.x), sqMin.y + 0.5f * (squareSize - textSize.y));
			
				drawList->AddText(font, fsize, text_pos, piece_color, pieceChar);
			}
		}
	}
	ImGui::End();
}

void pieceSelector(GameState& gameState, GuiState& guiState) {
	return;
}


void drawBitboards(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Bitboard viewer");

	ImGui::Combo("Bitboards", &guiState.bbSelectedIndex, BitboardStrings, IM_ARRAYSIZE(BitboardStrings));
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();

	const float squareSize = 40.0f;
	const int16 numColumns = 8;
	const int16 numRows = 8;

	const ImU32 gridColor = ImGui::GetColorU32(ImGuiCol_Border);
	const ImU32 frameColor = ImGui::GetColorU32(ImGuiCol_BorderShadow);
	const float gridThickness = 1.0f;
	const float frameThickness = 2.0f;

	ImVec2 headerMin = ImVec2(windowPos.x, windowPos.y);
	ImVec2 headerMax = ImVec2(windowPos.x + squareSize * numColumns, windowPos.y + 55);

	Bitboard bb = gameState.bitboards[guiState.bbSelectedIndex];

	ImVec2 boardMin = ImVec2(windowPos.x, headerMax.y);
	ImVec2 boardMax = ImVec2(windowPos.x + squareSize * numColumns, headerMax.y + squareSize * numRows);
	int16 boardHeight = numRows * squareSize;

	drawList->AddRectFilled(boardMin, boardMax, ImGui::GetColorU32(ImGuiCol_WindowBg));

	for (int16 row = 0; row < numRows; row++) {
		for (int16 col = 0; col < numColumns; col++) {
			ImVec2 sqMin = ImVec2(windowPos.x + col * squareSize, (boardHeight + headerMax.y) - row * squareSize);
			ImVec2 sqMax = ImVec2(windowPos.x + (col + 1) * squareSize, (boardHeight + headerMax.y) - (row + 1) * squareSize);
			uint8 index = (row * 8 + col);

			ImU32 fill = (bb & (1ULL << index)) ? BLACK_COLOR : WHITE_COLOR;

			drawList->AddRectFilled(sqMin, sqMax, fill);
			drawList->AddRect(sqMin, sqMax, gridColor, 0.0f, 0, gridThickness);
		}
	}

	drawList->AddRect(boardMin, boardMax, frameColor, 0.0f, 0, frameThickness);

	ImGui::End();
}

void drawToggles(GuiState& guiState) {

	ImGui::Checkbox("Show check mask", &guiState.showCheckMask);

	ImGui::Checkbox("Show pinned pieces", &guiState.showPinnedPieces);

	ImGui::Checkbox("Show pinnedRays", &guiState.showPinnedRays);
}

void drawUndoButton(GameState& gameState, GuiState& guiState) {

	if (ImGui::Button("Undo")) {
		if (guiState.movesMade.size() == 0) return;
		Move move = guiState.movesMade[guiState.movesMade.size() - 1];
		gameState.unmakeMove(move, guiState.history);
	}
}
