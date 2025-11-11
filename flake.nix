{
  description = "Nix-flake-based Qt/C++ development environment";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";
  outputs =
    { self, nixpkgs }:
    let
      eachSystem =
        f:
        let
          localSystems = [
            "aarch64-linux"
            "x86_64-linux"
            "aarch64-darwin"
          ];
          crossSystem = "aarch64-linux";
        in
        nixpkgs.lib.genAttrs localSystems (
          localSystem:
          f (import nixpkgs { inherit localSystem; }) (
            import nixpkgs {
              inherit localSystem;
              inherit crossSystem;
            }
          )
        );
    in
    {
      packages = eachSystem (
        pkgs: crossPkgs: {
          smart-piano = pkgs.qt5.callPackage ./smartPianoEngine.nix { inherit self; };
          cross-smart-piano = crossPkgs.qt5.callPackage ./smartPianoEngine.nix { inherit self; };
          default = self.packages.${pkgs.system}.smart-piano;
          cross = self.packages.${pkgs.system}.cross-smart-piano;
        }
      );
      devShells = eachSystem (
        pkgs: crossPkgs: {
          default = pkgs.mkShell {
            packages = with pkgs; [
              clang-tools # Clang CLIs, including LSP
              cmake-language-server # Cmake LSP
              # cppcheck # C++ Static analysis
              doxygen # Documentation generator
              fluidsynth # JACK Synthesizer
              soundfont-fluid # Soudfont for FluidSynth
              # gtest # Testing framework
              # lcov # Code coverage
              lldb # Clang debug adapter
              # valgrind # Debugging and profiling
            ];
            nativeBuildInputs = self.packages.${pkgs.system}.smart-piano.nativeBuildInputs;
            buildInputs = self.packages.${pkgs.system}.smart-piano.buildInputs;
            # Export compile commands JSON for LSP and other tools
            shellHook = ''
              mkdir --verbose build
              cd build
              cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
            '';
          };
        }
      );
    };
}
