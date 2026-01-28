{
  alsa-lib,
  cmake,
  doctest,
  lcov,
  llvm,
  libjack2,
  ninja,
  pkg-config,
  rtmidi,
  self,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "engine";
  version = "0.0.0";
  src = self;
  doCheck = true; # Enable tests
  nativeBuildInputs = [
    cmake # Modern build tool
    doctest # Testing framework
    lcov # Code coverage tool
    llvm # For llvm-cov
    ninja # Modern build tool
    pkg-config # Build tool
  ];
  buildInputs = [
    alsa-lib # Audio lib
    libjack2 # Audio interconnection lib
    rtmidi # MIDI lib
  ];
  cmakeFlags = [ "-DCODE_COVERAGE=ON" ];
  postBuild = ''
    cmake --build . --target coverage
  '';
  postInstall = ''
    mkdir -p $out/share/doc/coverage
    cp -r coverage_html/* $out/share/doc/coverage/
  '';
}
