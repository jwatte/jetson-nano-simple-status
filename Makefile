CPPFILES:=$(wildcard *.cpp)
OBJFILES:=$(patsubst %.cpp,obj/%.o,$(CPPFILES))
NAME:=jetson-nano-simple-status

jetson-nano-simple-status:	setup $(OBJFILES)
	g++ -o "$(NAME)" -g -Wall -Werror $(OBJFILES) -lfltk -lpthread
	@echo "you may want to 'make install' to install"

install:
	@if [ `whoami` = 'root' ]; then echo "\nPlease don't run install as root; it uses sudo when appropriate.\n"; exit 1; fi
	mkdir -p "$$HOME/.config/autostart"
	cp -fp "$(NAME).desktop" "$$HOME/.config/autostart/"
	sudo sh -c "cp -fp $(NAME).desktop /usr/share/applications/; cp -fp $(NAME) /usr/local/bin/; cp -fp $(NAME).png /usr/share/icons/hicolor/48x48/apps/"

clean:
	rm -rf obj "$(NAME)"

setup:
	[ -f /usr/lib/aarch64-linux-gnu/libfltk.so.1.3 ] || sudo apt install libfltk1.3-dev

obj/%.o:	%.cpp setup
	@mkdir -p obj
	g++ -MMD -c -std=c++14 -g -Wall -Werror -o $@ $<

.PHONY:	setup clean install

-include $(patsubst %.o,%.d,$(OBJFILES))
