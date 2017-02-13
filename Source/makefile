UNAME:=$(shell uname)

# Lua version in use
V?=53

# Lua libraries output directory
LUASOSDIR:= ./

# CPP drivers output directory
CPPDRVDIR:= driver/cpp

#---
# Darwin instructions for MacPorts on El Capitan
# Run: port install lua libpng libb64 freetype boost harfbuzz-icu
# Tweaks:
# lua53.pc missing.
#   Link from lua.pc to lua53.pc in /opt/local/lib/pkgconfig
# b64.pc mising.
#   Adjust it and copy to /opt/local/lib/pkgconfig
#---

SOLDFLAGS_Darwin:= -bundle -undefined dynamic_lookup
CP_Darwin=gcp

#---
# Linux instructions for Ubuntu 16.04
# Run: apt-get install liblua5.3-dev libboost-dev libharfbuzz-dev libb64-dev libfreetype6-dev
# Tweaks:
# Download and install libpng16 manually because it conflicts with libpng12.
# b64.pc missing.
#   Adjust it and copy to /usr/lib/x86_64-linux-gnu/pkgconfig/b64.pc
#---

SOLDFLAGS_Linux:= -shared -fpic
CP_Linux=cp

#---
# Probably no need to change anything below here
#---

CP=$(CP_$(UNAME))

# Load includes and libs from pkg-config
PKG:=pkg-config
LUAINC:=$(shell $(PKG) --cflags lua$V)
FTINC:=$(shell $(PKG) --cflags freetype2)
FTLIB:=$(shell $(PKG) --libs freetype2)
PNGINC:=$(shell $(PKG) --cflags libpng16)
PNGLIB:=$(shell $(PKG) --libs libpng16)
B64LIB=$(shell $(PKG) --libs b64)
B64INC=$(shell $(PKG) --cflags b64)
HBLIB=$(shell $(PKG) --libs harfbuzz-icu)
HBINC=$(shell $(PKG) --cflags harfbuzz-icu)

CC=gcc
CXX=g++
# Autmoatically generate dependencies
DEPFLAGS=-MT $@ -MMD -MP -MF $*.d
CXXFLAGS=-g -W -std=c++11 -Wall -fpic -fvisibility=hidden $(DEPFLAGS)
SOLDFLAGS:=$(SOLDFLAGS_$(UNAME))

# OBJS used in each SO library
IMAGEOBJS:=image/luaimage.o image/pngio.o compat/compat.o
BASE64OBJS:=base64/luabase64.o compat/compat.o
FTOBJS:=freetype/luafreetype.o compat/compat.o
VAOBJS:=vararg/luavararg.o compat/compat.o
CHRONOSOBJS:=chronos/luachronos.o chronos/chronos.o compat/compat.o
HBOBJS:=harfbuzz/luaharfbuzz.o compat/compat.o

# Common OBJS for the CPP drivers
DRVOBJS:= base64/base64.o chronos/chronos.o compat/compat.o description/description.o description/lua.o util/match.o image/pngio.o color/named.o color/color.o path/svg/command.o path/path.o xform/xform.o stroke/style.o xform/svd.o paint/paint.o

# OBJS for the CPP PNG driver
PNGDRVOBJS:= driver/cpp/png.o $(DRVOBJS)

# OBJS for the CPP CPP driver
CPPDRVOBJS:= driver/cpp/cpp.o $(DRVOBJS)

# OBJS for the CPP SVG driver
SVGDRVOBJS:= driver/cpp/svg.o $(DRVOBJS)

# List of all OBJS used anywhere
OBJS:=$(sort $(SVGDRVOBJS) $(CPPDRVOBJS) $(PNGDRVOBJS) $(IMAGEOBJS) $(BASE64OBJS) $(FTOBJS) $(VAOBJS) $(CHRONOSOBJS) $(HBOBJS))

# The dependency file for each each OBJ
DEPS:=$(OBJS:.o=.d)

# Lua libraries
LUASOS:= $(LUASOSDIR)/image.so $(LUASOSDIR)/base64.so $(LUASOSDIR)/freetype.so  $(LUASOSDIR)/chronos.so $(LUASOSDIR)/vararg.so  $(LUASOSDIR)/harfbuzz.so

# CPP drivers
CPPDRV:= $(CPPDRVDIR)/png.so $(CPPDRVDIR)/svg.so $(CPPDRVDIR)/cpp.so

# Targets that do not create files
.PHONY: all clean config

# Main target that generates all binaries
all: $(LUASOS) $(CPPDRV)

# Base of project is include directory
INC:= -I./

# Modify the include directories for each obj that needs them
image/luaimage.o: INC += $(LUAINC)
compat/compat.o: INC += $(LUAINC)
image/pngio.o: INC += $(PNGINC)
base64/luabase64.o: INC += $(LUAINC) $(B64INC)
base64/base64.o: INC += $(B64INC)
harfbuzz/luaharfbuzz.o: INC += $(LUAINC) $(FTINC) $(HBINC)
freetype/luafreetype.o: INC += $(LUAINC) $(FTINC)
chronos/luachronos.o: INC += $(LUAINC)
vararg/luavararg.o: INC += $(LUAINC)
driver/cpp/png.o: INC += $(LUAINC)
driver/cpp/svg.o: INC += $(LUAINC)
driver/cpp/cpp.o: INC += $(LUAINC)
description/lua.o: INC += $(LUAINC)

# Clear the pattern rule for OBJS from CPP
%.o: %.cpp

# Redefine pattern rule for OBJS from CPP
%.o: %.cpp %.d
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) $(INC) -o $@ -c $<

# Don't worry if dependency files are not found
$(DEPS): ;

# Do not delete dependency files or CPP samples
.PRECIOUS: $(DEPS)

# Include all dependency files to make them active
-include $(DEPS)

# Actual targets
vararg.so: $(VAOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(VAOBJS)

chronos.so: $(CHRONOSOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(CHRONOSOBJS)

image.so: $(IMAGEOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(IMAGEOBJS) $(PNGLIB)

base64.so: $(BASE64OBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(BASE64OBJS) $(B64LIB)

freetype.so: $(FTOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(FTOBJS) $(FTLIB)

harfbuzz.so: $(HBOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(HBOBJS) $(HBLIB) $(FTLIB)

$(CPPDRVDIR)/png.so: $(PNGDRVOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(PNGDRVOBJS) $(PNGLIB) $(B64LIB)

$(CPPDRVDIR)/svg.so: $(SVGDRVOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(SVGDRVOBJS) $(PNGLIB) $(B64LIB)

$(CPPDRVDIR)/cpp.so: $(CPPDRVOBJS)
	@echo linking $@
	@$(CXX) $(SOLDFLAGS) -o $@ $(CPPDRVOBJS) $(PNGLIB) $(B64LIB)

# Delete all automatically generated files
clean:
	\rm -f $(OBJS) $(DEPS)

# Show paths
config:
	@echo LUAINC: $(LUAINC)
	@echo LUALIB: $(LUALIB)
	@echo FTINC:  $(FTINC)
	@echo FTLIB:  $(FTLIB)
	@echo PNGINC: $(PNGINC)
	@echo PNGLIB: $(PNGLIB)
	@echo HBINC:  $(HBINC)
	@echo HBLIB:  $(HBLIB)
	@echo B64INC: $(B64INC)
	@echo B64LIB: $(B64LIB)
	@echo CP:     $(CP)

# Build the packages for distribution

DIST_LUA_SRC := \
	arc.lua \
	bernstein.lua \
	bezier.lua \
	bit/53.lua \
	bit/numberlua.lua \
	bit32.lua \
	blue.lua \
	circle.lua \
	color.lua \
	command.lua \
	commandtoinstruction.lua \
	description.lua \
	driver/lua/png.lua \
	driver/lua/rvg.lua \
	driver/lua/svg.lua \
	driver.lua \
	filter.lua \
	indent.lua \
	paint.lua \
	path.lua \
	polygon.lua \
	process.lua \
	profiler.lua \
	quadratic.lua \
	ramp.lua \
	rect.lua \
	scene.lua \
	spread.lua \
	strokable.lua \
	strokestyle.lua \
	svd.lua \
	text.lua \
	fonts.lua \
	triangle.lua \
	util.lua \
	viewport.lua \
	window.lua \
	xform.lua \
	xformable.lua

DIST_FONTS := \
	fonts/google \
	fonts/microsoft \
	fonts/urw

DIST_CPP_SRC := \
	base64/base64.cpp \
	base64/base64.h \
	base64/luabase64.cpp \
	base64/luabase64.h \
	bbox/bbox.h \
	bbox/viewport.h \
	bbox/window.h \
	bezier/bezier.h \
	chronos/chronos.cpp \
	chronos/chronos.h \
	chronos/luachronos.cpp \
	chronos/luachronos.h \
	color/color.cpp \
	color/color.h \
	color/named.cpp \
	color/named.h \
	compat/compat.cpp \
	compat/compat.h \
	description/description.cpp \
	description/description.h \
	description/lua.cpp \
	description/lua.h \
	description/painted.h \
	description/stencil.h \
	driver/cpp/cpp.cpp \
	driver/cpp/cpp.h \
	driver/cpp/png.cpp \
	driver/cpp/png.h \
	driver/cpp/svg.cpp \
	driver/cpp/svg.h \
	freetype/luafreetype.cpp \
	freetype/luafreetype.h \
	harfbuzz/luaharfbuzz.cpp \
	harfbuzz/luaharfbuzz.h \
	image/iimage.h \
	image/image.h \
	image/luaimage.cpp \
	image/luaimage.h \
	image/pngio.cpp \
	image/pngio.h \
	math/math.cpp \
	math/math.h \
	math/misc.cpp \
	math/polynomial.h \
	meta/meta.h \
	paint/lineargradient.h \
	paint/paint.cpp \
	paint/paint.h \
	paint/radialgradient.h \
	paint/ramp.h \
	paint/spread.h \
	paint/texture.h \
	path/datum.h \
	path/filter/bracket-lengths.h \
	path/filter/close-path.h \
	path/filter/forwarder.h \
	path/filter/null.h \
	path/filter/spy.h \
	path/filter/xformer.h \
	path/instruction.h \
	path/ipath.h \
	path/path.cpp \
	path/path.h \
	path/path.hpp \
	path/svg/command.cpp \
	path/svg/command.h \
	path/svg/filter/command-printer.h \
	path/svg/filter/command-to-instruction.h \
	path/svg/filter/instruction-to-command.h \
	path/svg/isvgpath.h \
	path/svg/parse.h \
	path/svg/token.cpp \
	path/svg/token.h \
	path/svg/tokenizer.h \
	point/ipoint.h \
	point/point.h \
	scene/iscene.h \
	scene/scene.h \
	scene/xformablescene.h \
	shape/circle.h \
	shape/polygon.h \
	shape/rect.h \
	shape/shape.h \
	shape/triangle.h \
	stroke/cap.h \
	stroke/dasharray.h \
	stroke/istrokable.h \
	stroke/join.h \
	stroke/main.cpp \
	stroke/method.h \
	stroke/style.cpp \
	stroke/style.h \
	test/test.h \
	util/indent.h \
	util/match.cpp \
	util/match.h \
	vararg/luavararg.cpp \
	xform/affinity.h \
	xform/affinity.hpp \
	xform/identity.h \
	xform/identity.hpp \
	xform/ixform.h \
	xform/linear.h \
	xform/linear.hpp \
	xform/mixed-product.hpp \
	xform/projectivity.h \
	xform/projectivity.hpp \
	xform/rotation.h \
	xform/rotation.hpp \
	xform/scaling.h \
	xform/scaling.hpp \
	xform/svd.cpp \
	xform/svd.h \
	xform/translation.h \
	xform/translation.hpp \
	xform/windowviewport.h \
	xform/xformable.h \
	xform/xform.cpp \
	xform/xform.h \
	xform/xform.hpp

DIST_PRJ := \
	makefile \
	README \
    b64.pc \
	base64.vcxproj \
	chronos.vcxproj \
	freetype.vcxproj \
	harfbuzz.vcxproj \
	image.vcxproj \
	vararg.vcxproj \
	vg.sln \
	cpp.vcxproj \
	svg.vcxproj \
	png.vcxproj \
    paths.props

DIST_RVGS := \
	rvgs/ampersand.rvg \
	rvgs/anatomical_heart.rvg \
	rvgs/arc1.rvg \
	rvgs/arc2.rvg \
	rvgs/arc3.rvg \
	rvgs/arc4.rvg \
	rvgs/arc5.rvg \
	rvgs/blue_butterfly.rvg \
	rvgs/blur.rvg \
	rvgs/bunny.rvg \
	rvgs/carrera.rvg \
	rvgs/circle.rvg \
	rvgs/clippath1.rvg \
	rvgs/clippath2.rvg \
	rvgs/clippath3.rvg \
	rvgs/clippath4.rvg \
	rvgs/clippath5.rvg \
	rvgs/clippath6.rvg \
	rvgs/clippath7.rvg \
	rvgs/clippath8.rvg \
	rvgs/clippath9.rvg \
	rvgs/cubic1.rvg \
	rvgs/cubic2.rvg \
	rvgs/cubic3.rvg \
	rvgs/cubic4.rvg \
	rvgs/cubic5.rvg \
	rvgs/cubic6.rvg \
	rvgs/cubic7.rvg \
	rvgs/cubic8.rvg \
	rvgs/cubic9.rvg \
	rvgs/dancer.rvg \
	rvgs/drops.rvg \
	rvgs/embrace.rvg \
	rvgs/eopolygon.rvg \
	rvgs/eyes.rvg \
	rvgs/hello_ttf.rvg \
	rvgs/hello_type1.rvg \
	rvgs/hole1.rvg \
	rvgs/hole2.rvg \
	rvgs/hole3.rvg \
	rvgs/hole4.rvg \
	rvgs/icozahedron.rvg \
	rvgs/leopard.rvg \
	rvgs/lineargradient.rvg \
	rvgs/lion.rvg \
	rvgs/nestedblur.rvg \
	rvgs/nestedclippath.rvg \
	rvgs/nestedxform1.rvg \
	rvgs/nestedxform2.rvg \
	rvgs/page_1.rvg \
	rvgs/page_2.rvg \
	rvgs/page.rvg \
	rvgs/parabola1.rvg \
	rvgs/parabola2.rvg \
	rvgs/parabola3.rvg \
	rvgs/parabola4.rvg \
	rvgs/parabola5.rvg \
	rvgs/parabola6.rvg \
	rvgs/patheopolygon.rvg \
	rvgs/pathlion.rvg \
	rvgs/pathpolygon.rvg \
	rvgs/pathtriangle1.rvg \
	rvgs/pathtriangle2.rvg \
	rvgs/pathtriangle3.rvg \
	rvgs/pathtriangle4.rvg \
	rvgs/penguin.rvg \
	rvgs/polygon.rvg \
	rvgs/quad1.rvg \
	rvgs/quad2.rvg \
	rvgs/quad3.rvg \
	rvgs/quad4.rvg \
	rvgs/radialgradient.rvg \
	rvgs/ramp1.rvg \
	rvgs/ramp2.rvg \
	rvgs/ramp3.rvg \
	rvgs/ramp4.rvg \
	rvgs/ramp5.rvg \
	rvgs/ramp6.rvg \
	rvgs/relatorio.rvg \
	rvgs/reschart.rvg \
	rvgs/stroke1.rvg \
	rvgs/stroke2.rvg \
	rvgs/stroke3.rvg \
	rvgs/stroke4.rvg \
	rvgs/stroke5.rvg \
	rvgs/svgarc1.rvg \
	rvgs/svgarc2.rvg \
	rvgs/svgarc3.rvg \
	rvgs/texture.rvg \
	rvgs/transparency.rvg \
	rvgs/transparentlineargradient.rvg \
	rvgs/transparentradialgradient.rvg \
	rvgs/triangle1.rvg \
	rvgs/triangle2.rvg \
	rvgs/triangulatedlion.rvg \
	rvgs/windowviewport1.rvg \
	rvgs/windowviewport2.rvg \
	rvgs/xformedblur1.rvg \
	rvgs/xformedblur2.rvg \
	rvgs/xformedcircle.rvg \
	rvgs/xformedlineargradient.rvg \
	rvgs/xformedradialgradient.rvg \
	rvgs/xformedtriangle.rvg

DIST_WIN64_REL_BIN := \
	~/build/vc14/bin/lua/5.3/x64/Release/lua.exe \
	~/build/vc14/bin/lua/5.3/x64/Release/lualib.dll \
	x64/Release/base64.dll \
	x64/Release/chronos.dll \
	x64/Release/freetype.dll \
	x64/Release/harfbuzz.dll \
	x64/Release/image.dll \
	x64/Release/vararg.dll

DIST_WIN64_DBG_BIN := \
	~/build/vc14/bin/lua/5.3/x64/Debug/lua.exe \
	~/build/vc14/bin/lua/5.3/x64/Debug/lualib.dll \
	x64/Debug/base64.dll \
	x64/Debug/chronos.dll \
	x64/Debug/freetype.dll \
	x64/Debug/harfbuzz.dll \
	x64/Debug/image.dll \
	x64/Debug/vararg.dll

DIST_WIN32_REL_BIN := \
	~/build/vc14/bin/lua/5.3/Release/lua.exe \
	~/build/vc14/bin/lua/5.3/Release/lualib.dll \
	Release/base64.dll \
	Release/chronos.dll \
	Release/freetype.dll \
	Release/harfbuzz.dll \
	Release/image.dll \
	Release/vararg.dll

DIST_WIN32_DBG_BIN := \
	~/build/vc14/bin/lua/5.3/Debug/lua.exe \
	~/build/vc14/bin/lua/5.3/Debug/lualib.dll \
	Debug/base64.dll \
	Debug/chronos.dll \
	Debug/freetype.dll \
	Debug/harfbuzz.dll \
	Debug/image.dll \
	Debug/vararg.dll

DIST_WIN32_VER := 1.04
DIST_WIN32_ZIP := win32-$(DIST_WIN32_VER).zip
DIST_WIN32_DIR := win32-$(DIST_WIN32_VER)

DIST_WIN64_VER := 1.04
DIST_WIN64_ZIP := win64-$(DIST_WIN64_VER).zip
DIST_WIN64_DIR := win64-$(DIST_WIN64_VER)

DIST_RVGS_VER := 1.04
DIST_RVGS_ZIP := rvgs-$(DIST_RVGS_VER).zip
DIST_RVGS_DIR := rvgs-$(DIST_RVGS_VER)

DIST_SRC_VER := 1.06
DIST_SRC_ZIP := src-$(DIST_SRC_VER).zip
DIST_SRC_DIR := src-$(DIST_SRC_VER)

DIST_FONTS_VER := 1.0
DIST_FONTS_ZIP := fonts-$(DIST_FONTS_VER).zip
DIST_FONTS_DIR := fonts-$(DIST_FONTS_VER)

.PHONY: versions dist distsrc distwin32 distwin64 distrvgs distfonts

distsrc: dist/$(DIST_SRC_ZIP)
distfonts: dist/$(DIST_FONTS_ZIP)
distwin32: dist/$(DIST_WIN32_ZIP)
distwin64: dist/$(DIST_WIN64_ZIP)
distrvgs: dist/$(DIST_RVGS_ZIP)

versions:
	@echo $(DIST_SRC_ZIP) $(DIST_WIN32_ZIP) $(DIST_WIN64_ZIP) $(DIST_RVGS_ZIP) $(DIST_FONTS_ZIP)

dist: distwin32 distwin64 distrvgs distsrc distfonts

dist/$(DIST_WIN32_ZIP): $(DIST_WIN32_BIN)
	mkdir -p dist/$(DIST_WIN32_DIR)
	mkdir -p dist/$(DIST_WIN32_DIR)/Release
	$(CP) $(DIST_WIN32_REL_BIN) dist/$(DIST_WIN32_DIR)/Release
	mkdir -p dist/$(DIST_WIN32_DIR)/Debug
	$(CP) $(DIST_WIN32_DBG_BIN) dist/$(DIST_WIN32_DIR)/Debug
	cd dist && zip -r $(DIST_WIN32_ZIP) $(DIST_WIN32_DIR)

dist/$(DIST_WIN64_ZIP): $(DIST_WIN64_BIN)
	mkdir -p dist/$(DIST_WIN64_DIR)
	mkdir -p dist/$(DIST_WIN64_DIR)/Release
	$(CP) $(DIST_WIN64_REL_BIN) dist/$(DIST_WIN64_DIR)/Release
	mkdir -p dist/$(DIST_WIN64_DIR)/Debug
	$(CP) $(DIST_WIN64_DBG_BIN) dist/$(DIST_WIN64_DIR)/Debug
	cd dist && zip -r $(DIST_WIN64_ZIP) $(DIST_WIN64_DIR)

dist/$(DIST_RVGS_ZIP): $(DIST_RVGS)
	mkdir -p dist
	mkdir dist/$(DIST_RVGS_DIR)
	$(CP) -f $(DIST_RVGS) dist/$(DIST_RVGS_DIR)
	cd dist && zip -r $(DIST_RVGS_ZIP) $(DIST_RVGS_DIR)

dist/$(DIST_FONTS_ZIP):
	mkdir -p dist
	mkdir dist/$(DIST_FONTS_DIR)
	$(CP) -fr $(DIST_FONTS) dist/$(DIST_FONTS_DIR)
	cd dist && zip -r $(DIST_FONTS_ZIP) $(DIST_FONTS_DIR)

dist/$(DIST_SRC_ZIP): $(DIST_LUA_SRC) $(DIST_CPP_SRC) $(DIST_PRJ)
	mkdir -p dist
	mkdir dist/$(DIST_SRC_DIR)
	$(CP) --parents $(DIST_LUA_SRC) dist/$(DIST_SRC_DIR)
	$(CP) --parents $(DIST_PRJ) dist/$(DIST_SRC_DIR)
	$(CP) --parents $(DIST_CPP_SRC) dist/$(DIST_SRC_DIR)
	cd dist && zip -r $(DIST_SRC_ZIP) $(DIST_SRC_DIR)
