CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./chess -I./movegen -I./helpers

OBJS = main.o \
	chess/GameRules.o \
	chess/GameState.o \
	chess/Move.o \
	movegen/MoveGen.o \
	movegen/MoveGenTest.o \
	helpers/GameStateHelper.o \
	search/Evaluation.o \
	search/EvaluationTests.o \
	search/Search.o

release: CXXFLAGS += -O2
release: TARGET = engine
release: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	rm -f $(OBJS)

debug: CXXFLAGS += -g -DDEBUG_MODE
debug: TARGET = engine_debug
debug: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	rm -f $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) engine engine_debug
