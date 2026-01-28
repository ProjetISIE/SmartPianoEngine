{
  alsa-lib,
  cmake,
  doctest,
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
    llvm # For llvm-cov
    ninja # Modern build tool
    pkg-config # Build tool
  ];
  buildInputs = [
    alsa-lib # Audio lib
    libjack2 # Audio interconnection lib
    rtmidi # MIDI lib
  ];
}
