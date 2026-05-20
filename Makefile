CXX = g++
CXXFLAGS = -Wshadow -Wswitch-enum -Winit-self -Wredundant-decls -Wcast-align \
           -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations \
           -Wmissing-include-dirs -Wswitch-default -Weffc++ -Wmain -Wextra -Wall \
           -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy \
           -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers \
           -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor \
           -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 \
           -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings \
           -Werror=vla -D _DEBUG -D _EJUDGE_CLIENT_SIDE -fsanitize=address

COMMON_FILES = Taras_Bulba.cpp file_using.cpp dump.cpp

FRONTEND_FILES = $(COMMON_FILES) infix_read.cpp front_end_main.cpp output.cpp
FRONTEND_TARGET = front_end.out

BACKEND_FILES = $(COMMON_FILES) back_end.cpp back_end_main.cpp input.cpp
BACKEND_TARGET = back_end.out

.PHONY: all front_end back_end asm_test clean clean-all help

all: $(FRONTEND_TARGET) $(BACKEND_TARGET)

front_end: $(FRONTEND_TARGET)

back_end: $(BACKEND_TARGET)

$(FRONTEND_TARGET): $(FRONTEND_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BACKEND_TARGET): $(BACKEND_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o

clean-all: clean
	rm -f $(FRONTEND_TARGET) $(BACKEND_TARGET)

clean-exec:
	rm -f $(FRONTEND_TARGET) $(BACKEND_TARGET)

help:
	@echo "Доступные цели:"
	@echo "  make            - сборка front_end и back_end (по умолчанию)"
	@echo "  make all        - то же самое"
	@echo "  make front_end  - сборка только фронтенда"
	@echo "  make back_end   - сборка только бэкенда"
	@echo "  make clean      - удаление .o файлов"
	@echo "  make clean-exec - удаление только исполняемых файлов"
	@echo "  make clean-all  - полная очистка (.o + исполняемые)"
