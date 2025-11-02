#include <vector>
#include <sstream>

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
						if (move.getStartSquare() == guiState.selectedPieceSq) guiState.selectedPieceMoves.push(move);
					}
				}
				else {
					for (Move& move : guiState.selectedPieceMoves) {
						if (index == move.getTargetSquare() && guiState.selectedPieceSq == move.getStartSquare()) {
							updateEval(gameState, move, gameState.colorToMove, guiState.eval, guiState.evalStack);
							gameState.makeMove(move, guiState.history);
							guiState.movesMade.push_back(move);

							guiState.selectedPieceSq = -1;
							guiState.selectedPieceMoves.clear();
							guiState.allMoves.clear();

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

			drawList->AddRectFilled(sqMin, sqMax, color);
		}
	}

	if (guiState.showPinnedRays) {
		for (uint8 i = 0; i < 64; i++) {
			if (guiState.pinnedRays[i] != 0) {
				Bitboard rayBB = guiState.pinnedRays[i];
				while (rayBB) {
					uint8 sq = __builtin_ctzll(rayBB);
					ImVec2 sqMin = ImVec2(windowPos.x + (sq % 8) * squareSize, headerMax.y + (numRows - 1 - (sq / 8)) * squareSize);
					ImVec2 sqMax = ImVec2(sqMin.x + squareSize, sqMin.y + squareSize);

					drawList->AddRectFilled(sqMin, sqMax, PINNED_RAYS_COLOR);
					rayBB &= rayBB - 1;
				}
			}
		}
		
	}

	for (int16 row = 0; row < numRows; row++) {
		for (int16 col = 0; col < numCols; col++) {

			ImVec2 sqMin = ImVec2(windowPos.x + col * squareSize, headerMax.y + (numRows - 1 - row) * squareSize);
			uint8 index = (row * numCols + col);

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
	
	ImVec2 boardMin = ImVec2(windowPos.x, headerMax.y);
	ImVec2 boardMax = ImVec2(windowPos.x + squareSize * numCols, headerMax.y + boardHeight);

	float borderThickness = 2.0f;
	drawList->AddRect(boardMin, boardMax, BLACK_COLOR, 0.0f, 0, borderThickness);

	ImFont* font = ImGui::GetFont();
	float base = ImGui::GetFontSize();
	float labelSize = base * 0.9f;

	for (int16 col = 0; col < numCols; ++col) {
		char fileChar[2] = { static_cast<char>('a' + col), '\0' };

		ImVec2 sqCenterBottom = ImVec2( windowPos.x + col * squareSize + squareSize * 0.5f, headerMax.y + boardHeight + 2.0f);
		ImVec2 fileSize = font->CalcTextSizeA(labelSize, FLT_MAX, 0.0f, fileChar);
		ImVec2 filePosBottom = ImVec2(sqCenterBottom.x - fileSize.x * 0.5f, sqCenterBottom.y);

		ImVec2 sqCenterTop = ImVec2(windowPos.x + col * squareSize + squareSize * 0.5f, headerMax.y - fileSize.y - 2.0f);
		ImVec2 filePosTop = ImVec2(sqCenterTop.x - fileSize.x * 0.5f, sqCenterTop.y);

		drawList->AddText(font, labelSize, filePosBottom, WHITE_COLOR, fileChar);
		drawList->AddText(font, labelSize, filePosTop, WHITE_COLOR, fileChar);
	}

	for (int16 row = 0; row < numRows; ++row) {
		char rankChar[2] = { static_cast<char>('1' + row), '\0' };

		float yTopOfRow = headerMax.y + (numRows - 1 - row) * squareSize;
		float yCenter = yTopOfRow + squareSize * 0.5f;

		ImVec2 rankSize = font->CalcTextSizeA(labelSize, FLT_MAX, 0.0f, rankChar);

		ImVec2 leftPos = ImVec2(boardMin.x - rankSize.x - 4.0f, yCenter - rankSize.y * 0.5f);
		ImVec2 rightPos = ImVec2(boardMax.x + 4.0f, yCenter - rankSize.y * 0.5f);

		drawList->AddText(font, labelSize, leftPos, WHITE_COLOR, rankChar);
		drawList->AddText(font, labelSize, rightPos, WHITE_COLOR, rankChar);
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
		undoEvalUpdate(guiState.eval, guiState.evalStack);
		guiState.movesMade.pop_back();
		guiState.allMoves.clear();
		generateAllMoves(gameState, guiState.allMoves, gameState.colorToMove, guiState.checkMask, guiState.pinnedPieces, guiState.pinnedRays);
	}
}

void drawSelectPinnedRayIndex(GuiState& guiState) {
	ImGui::Begin("Pinned ray index");

	if (ImGui::BeginTable("Pinned ray index", 8, ImGuiTableFlags_SizingFixedFit)) {

		for (int row = 0; row < 8; ++row) {
			ImGui::TableNextRow();
			for (int col = 0; col < 8; ++col) {
				int v = (7 - row) * 8 + col;

				ImGui::TableSetColumnIndex(col);
				ImGui::PushID(v);

				ImGuiSelectableFlags selectFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
				bool pressed = ImGui::Selectable(std::to_string(v).c_str(), guiState.selectedRayIndex == v, selectFlags, ImVec2(12, 12));
				ImGui::PopID();

				if (pressed) guiState.selectedRayIndex = v;
			}
		}
		ImGui::EndTable();
	}

	ImGui::End();
}

void drawPinnedRayBitboard(GuiState& guiState) {
	ImGui::Begin("Pinned ray viewer");

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
	ImVec2 headerMax = ImVec2(windowPos.x + squareSize * numColumns, windowPos.y + 20);

	Bitboard bb = guiState.pinnedRays[guiState.selectedRayIndex];

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

void drawIsSquareAttacked(GameState& gameState) {
	static int32 sq = 0;
	ImGui::Begin("Is Square Attacked?");
	ImGui::InputInt("index", &sq);
	Color them = gameState.colorToMove == White ? Black : White;
	ImGui::Text(isSquareAttacked(gameState, 1ULL << sq, them) ? "TRUE" : "FALSE");
	ImGui::End();
}


void drawEval(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Static Eval vs Incremental Eval");

	auto drawEvalTable = [](const char* tableId, const EvalState& ev) {

		auto rowSides = [](const char* label, const int16 (&arr)[2]) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
			ImGui::TableNextColumn(); ImGui::Text("%hd", arr[White]);
			ImGui::TableNextColumn(); ImGui::Text("%hd", arr[Black]);
			ImGui::TableNextColumn(); ImGui::Text("%d", int(arr[White]) - int(arr[Black]));
		};

		if (ImGui::BeginTable(tableId, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableSetupColumn("Category");
			ImGui::TableSetupColumn("White");
			ImGui::TableSetupColumn("Black");
			ImGui::TableSetupColumn("Net (W-B)");
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::TextUnformatted("Phase");
			ImGui::TableNextColumn(); ImGui::Text("%hd", ev.phase);
			ImGui::TableNextColumn(); ImGui::Text("%hd", TOTAL_PHASE - ev.phase);
			ImGui::TableNextColumn(); ImGui::Text("%hd", TOTAL_PHASE);

			rowSides("MG Score", ev.mgSide);
			rowSides("EG Score", ev.egSide);

			rowSides("Pawn Structure", ev.pawnStructure);
			rowSides("King Safety", ev.kingSafety);
			rowSides("Bishop Pair", ev.bishopPair);
			rowSides("Knight Pair", ev.knightPair);
			rowSides("Rook Pair", ev.rookPair);
			rowSides("Knight Adj", ev.knightAdj);
			rowSides("Rook Adj", ev.rookAdj);

			// rowSides("Knight Mobility", ev.knightMobility);
			// rowSides("Bishop Mobility", ev.bishopMobility);
			// rowSides("Rook Mobility", ev.rookMobility);
			// rowSides("Queen Mobility", ev.queenMobility);

			ImGui::EndTable();
		}
	};

	{
		std::stringstream ss;
		EvalState staticEval{};
		initEval(gameState, staticEval, gameState.colorToMove);
		ss << "Static Eval: " << getEval(staticEval, gameState.colorToMove);
		ImGui::Text("%s", ss.str().c_str());
		drawEvalTable("Static Eval Details", staticEval);
	}

	{
		std::stringstream ss;
		ss << "Incremental Eval: " << getEval(guiState.eval, gameState.colorToMove);
		ImGui::Text("%s", ss.str().c_str());
		drawEvalTable("Incremental Eval Details", guiState.eval);
	}

	ImGui::End();
}

