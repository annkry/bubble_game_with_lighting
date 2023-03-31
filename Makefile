default: main

%: %.cpp
	g++ -I. $< -o $@ shader.cpp -lGLEW -lGL -lglfw
clean:
	rm a.out *.o *~ main
