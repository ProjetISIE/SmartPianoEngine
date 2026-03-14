{
  description = "Nix flake C++23 development environment";
  nixConfig = {
    extra-substituters = [ "https://cache.garnix.io" ];
    extra-trusted-public-keys = [ "cache.garnix.io:CTFPyKSLcx5RMJKfLo5EEPUObbA78b0YQ2DTCJXqr9g=" ];
  };
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux" # "aarch64-linux"
        "aarch64-darwin"
      ];
      forSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          makeEngine =
            p:
            p.callPackage ./engine.nix {
              stdenv = p.clangStdenv;
            };
        in
        {
          default = makeEngine pkgs;
          smart-piano-engine = makeEngine pkgs;
        }
        // nixpkgs.lib.optionalAttrs (system == "x86_64-linux") {
          smart-piano-engine-aarch64 = makeEngine pkgs.pkgsCross.aarch64-multiplatform;
        }
      );
      checks = forSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          defaultPkg = self.packages.${system}.default;
        in
        {
          coverage = pkgs.clangStdenv.mkDerivation {
            name = "coverage-check";
            src = ./.;
            doCheck = true;
            nativeBuildInputs = defaultPkg.nativeBuildInputs ++ [
              pkgs.lcov
              pkgs.jq
            ];
            buildInputs = defaultPkg.buildInputs;
            configurePhase = ''
              cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON -GNinja -S . -B build
            '';
            buildPhase = ''
              cmake --build build -j16
            '';
            checkPhase = ''
              cmake --build build --target coverage -j1 # Multithread breaks tests
              # Text report for the CI comment
              llvm-cov report build/src/main -instr-profile=build/coverage.profdata -ignore-filename-regex='test/.*' > build/coverage.txt
              cat build/coverage.txt
              # JSON export for threshold verification
              llvm-cov export build/src/main -instr-profile=build/coverage.profdata -ignore-filename-regex='test/.*' --summary-only > build/coverage.json
            '';
            installPhase = ''
              mkdir -p $out
              if [ -f "build/coverage.txt" ]; then cp build/coverage.txt $out/; fi
              if [ -f "build/coverage.json" ]; then cp build/coverage.json $out/; fi
              if [ -d "build/coverage" ]; then cp -R build/coverage $out/html; fi
            '';
          };
        }
      );
      devShells = forSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          defaultPkg = self.packages.${system}.default;
        in
        {
          default =
            pkgs.mkShell.override
              {
                stdenv = pkgs.clangStdenv; # Clang instead of GCC
              }
              {
                packages = with pkgs; [
                  bashInteractive
                  clang-tools # Clang CLIs, including LSP
                  clang-uml # UML diagram generator
                  cmake-format # CMake formatter
                  cmake-language-server # Cmake LSP
                  doxygen # Documentation generator
                  fluidsynth # JACK Synthesizer
                  lldb # Clang debug adapter
                  qsynth # FluidSynth GUI
                  socat # Serial terminal for manual testing
                  valgrind # Debugging and profiling
                ];
                inputsFrom = [ defaultPkg ];
                # Export compile commands JSON for LSP and other tools
                shellHook = ''
                  export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath defaultPkg.buildInputs}:$LD_LIBRARY_PATH"
                  cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug \
                    -DCOVERAGE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
                '';
              };
        }
      );
    };
}
