{
  alsa-lib,
  cmake,
  doctest,
  stdenv,
  ninja,
  pkg-config,
  rtmidi,
  self,
}:
stdenv.mkDerivation {
  pname = "engine";
  version = "0.0.0";
  src = self;
  doCheck = true; # Enable tests
  nativeBuildInputs = [
    cmake # Modern build tool
    doctest # Testing framework
    ninja # Modern build tool
    pkg-config # Build tool
  ];
  buildInputs = [
    alsa-lib # Audio lib
    # libjack2 # Audio interconnection lib
    rtmidi # MIDI lib
  ];
}
