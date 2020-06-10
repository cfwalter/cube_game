SDLFLAGS = $(shell sdl2-config --libs --cflags)
COMPILER_FLAGS= -F/Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -framework CoreFoundation -D_THREAD_SAFE -std=c++17
PROGRAM=Cube
OUTPUTPATH=build/Release
BUNDLE=$(OUTPUTPATH)/$(PROGRAM).app

game:
	mkdir -p build
	rm -f build/main
	g++ src/*.cpp -o build/main $(COMPILER_FLAGS)

release:
	rm -rf $(OUTPUTPATH)
	mkdir -p $(OUTPUTPATH)
	mkdir -p $(BUNDLE)/Contents
	mkdir -p $(BUNDLE)/Contents/MacOS
	mkdir -p $(BUNDLE)/Contents/Resources
	mkdir -p $(BUNDLE)/Contents/Frameworks
	cp -RH /Library/Frameworks/SDL2.framework $(BUNDLE)/Contents/Frameworks/
	cp -RH /Library/Frameworks/SDL2_image.framework $(BUNDLE)/Contents/Frameworks/
	cp -RH /Library/Frameworks/SDL2_mixer.framework $(BUNDLE)/Contents/Frameworks/
	cp -RH /Library/Frameworks/SDL2_ttf.framework $(BUNDLE)/Contents/Frameworks/
	cp -r Resources $(BUNDLE)/Contents
	g++ src/*.cpp -o $(BUNDLE)/Contents/MacOS/$(PROGRAM) $(COMPILER_FLAGS)
	install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2             @executable_path/../Frameworks/SDL2.framework/Versions/A/SDL2 $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change @rpath/SDL2_image.framework/Versions/A/SDL2_image @executable_path/../Frameworks/SDL2_image.framework/Versions/A/SDL2_image $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change @rpath/SDL2_mixer.framework/Versions/A/SDL2_mixer @executable_path/../Frameworks/SDL2_mixer.framework/Versions/A/SDL2_mixer $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change @rpath/SDL2_ttf.framework/Versions/A/SDL2_ttf     @executable_path/../Frameworks/SDL2_ttf.framework/Versions/A/SDL2_ttf $(BUNDLE)/Contents/MacOS/$(PROGRAM)
