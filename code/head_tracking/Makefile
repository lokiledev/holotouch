TARGET= facedetect
SRC= facedetect.c
$(TARGET): $(SRC)
	gcc -ggdb `pkg-config opencv --cflags --libs` $(SRC) -o $(TARGET) `pkg-config --libs opencv`;