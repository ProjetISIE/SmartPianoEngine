{
  description = "Nix flake C++23 development environment";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      systems =
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
      packages = systems (
        pkgs: crossPkgs: rec {
          smart-piano = pkgs.callPackage ./engine.nix { inherit self; };
          cross-smart-piano = crossPkgs.callPackage ./engine.nix { inherit self; };
          default = smart-piano;
          cross = cross-smart-piano;
        }
      );
      devShells = systems (
        pkgs: crossPkgs: {
          default =
            pkgs.mkShell.override
              {
                stdenv = pkgs.clangStdenv; # Clang instead of GCC
              }
              {
                packages = with pkgs; [
                  clang-tools # Clang CLIs, including LSP
                  clang-uml # UML diagram generator
                  cmake-format # CMake formatter
                  cmake-language-server # Cmake LSP
                  # cppcheck # C++ Static analysis
                  doxygen # Documentation generator
                  # fluidsynth # JACK Synthesizer
                  # gtest # Testing framework
                  lcov # Code coverage
                  lldb # Clang debug adapter
                  llvm # For llvm-cov
                  # neocmakelsp # CMake LSP
                  # qsynth # FluidSynth GUI
                  socat # Serial terminal for manual testing
                  # valgrind # Debugging and profiling
                ];
                nativeBuildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.nativeBuildInputs;
                buildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.buildInputs;
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
