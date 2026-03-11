{
  cmake,
  cppcheck,
  doctest,
  llvm,
  ninja,
  pkg-config,
  pkgs,
  rtmidi,
  self,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "engine";
  version = "0.1.0";
  src = self;
  doCheck = false; # Already in check coverage
  nativeBuildInputs = [
    # clang # C/C++ compiler
    cmake # Modern build tool
    cppcheck # C++ Static analysis
    doctest # Testing framework
    llvm # For llvm-cov
    ninja # Modern build tool
    pkg-config # Build tool
  ];
  buildInputs = [
    rtmidi # MIDI lib
  ]
  ++ pkgs.lib.optionals stdenv.isLinux [
    pkgs.alsa-lib # Audio lib
    pkgs.libjack2 # Audio interconnection lib
  ];
  installPhase = ''
    mkdir --parents --verbose $out/include
    cp --recursive --verbose ${./include}/* $out/include/
    mkdir --parents --verbose $out/lib
    cp --verbose src/libenginecomm.a $out/lib/
    mkdir --parents --verbose $out/bin
    cp --verbose src/main $out/bin/engine
  '';
}
