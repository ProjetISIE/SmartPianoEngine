{
  lib,
  stdenv,
  cmake,
  cppcheck,
  doctest,
  llvm,
  ninja,
  pkg-config,
  rtmidi,
  alsa-lib,
  libjack2,
}:
stdenv.mkDerivation {
  pname = "engine";
  version = "0.1.0";
  src = lib.cleanSource ./.;
  nativeBuildInputs = [
    cmake # Modern build tool
    cppcheck # C++ Static analysis
    llvm # For llvm-cov
    ninja # Modern build tool
    pkg-config # Build tool
  ];
  buildInputs = [
    doctest # Testing framework
    rtmidi # MIDI lib
  ]
  ++ lib.optionals stdenv.isLinux [
    alsa-lib # Audio lib
    libjack2 # Audio interconnection lib
  ];
  installPhase = ''
    runHook preInstall
    mkdir --parents --verbose $out/include
    cp --recursive --verbose ${./include}/* $out/include/
    mkdir --parents --verbose $out/lib
    cp --verbose src/libenginecomm.a $out/lib/
    mkdir --parents --verbose $out/bin
    cp --verbose src/main $out/bin/engine
    runHook postInstall
  '';
}
