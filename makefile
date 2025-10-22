CXX      = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -I./chess -I./movegen -I./helpers

SDL2_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL2_LIBS   := $(shell pkg-config --libs sdl2)

OBJDIR := obj

IMGUI_DIR      := external/imgui
IMGUI_BACKENDS := $(IMGUI_DIR)/backends

IMGUI_RAW := \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_BACKENDS)/imgui_impl_sdl2.cpp \
	$(IMGUI_BACKENDS)/imgui_impl_sdlrenderer2.cpp

IMGUI_OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(IMGUI_RAW))

RAW_OBJS = main.o \
	chess/GameRules.o \
	chess/GameState.o \
	chess/Move.o \
	movegen/MoveGen.o \
	movegen/MoveGenTest.o \
	helpers/GameStateHelper.o \
	helpers/Perft.o \
	search/Evaluation.o \
	search/EvaluationTests.o \
	search/MoveSorter.o \
	search/Search.o

OBJS := $(addprefix $(OBJDIR)/,$(RAW_OBJS))

RAW_GUI_OBJS := $(filter-out main.o,$(RAW_OBJS)) gui/BoardView.o guiMain.o
GUI_OBJS := $(addprefix $(OBJDIR)/,$(RAW_GUI_OBJS)) $(IMGUI_OBJS)

.PHONY: all debug release debug_eval gui obj clean

all: debug

debug: CXXFLAGS += -g -DDEBUG_MODE
debug: TARGET = engine-debug
debug: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

release: CXXFLAGS += -O2 -DUCI_MODE
release: TARGET = engine
release: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

debug_eval: CXXFLAGS += -g -DDEBUG_EVAL
debug_eval: TARGET = engine-debug-eval
debug_eval: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

gui: TARGET = chess-gui
gui: CXXFLAGS += $(SDL2_CFLAGS) -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS)
gui: $(GUI_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(GUI_OBJS) $(SDL2_LIBS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	@mkdir -p $(sort $(dir $(OBJS) $(GUI_OBJS)))

clean:
	rm -rf $(OBJDIR) engine engine-debug engine-debug-eval chess-gui
