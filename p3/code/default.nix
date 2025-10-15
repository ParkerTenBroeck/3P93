{ pkgs ? (import <nixpkgs> { 
    config.allowUnfree = true;
    config.impureUseNativeOptimizations = true;
    config.segger-jlink.acceptLicense = true; 
}), ... }:
pkgs.mkShell
{
	buildInputs = with pkgs; [
		cmake 
		gcc
		clang
		emscripten
		ninja
		#customPkgs.jetbrains.clion
		jetbrains.clion
		#clion = import ~/my-nixpkgs/pkgs/applications/editors/jetbrains {};
		renderdoc
		llvmPackages.openmp
		valgrind
	];
	propagatedBuildInputs = with pkgs; [
		xorg.libX11 
		xorg.libX11.dev
		xorg.libXcursor
		xorg.libXcursor.dev
		xorg.libXext 
		xorg.libXext.dev
		xorg.libXinerama
		xorg.libXinerama.dev 
		xorg.libXrandr 
		xorg.libXrandr.dev
		xorg.libXrender
		xorg.libXrender.dev
		xorg.libxcb
		xorg.libxcb.dev
		xorg.libXi
		xorg.libXi.dev
		harfbuzz
		harfbuzz.dev
		zlib
		zip
		zlib.dev
		bzip2
		bzip2.dev
		pngpp
		brotli
		brotli.dev
		ffmpeg
		pulseaudio.dev
		git
		libGL
		libGL.dev
		glfw
	];
#	shellHook = """
#	    unset NIX_ENFORCE_NO_NATIVE
#	""";
    NIX_ENFORCE_NO_NATIVE="";
	LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib";
}
