{
  description = "A Nix-flake-based C/C++ development environment";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05"; # NixOS Stable
  # inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  # inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";
  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" ];
      forEachSupportedSystem = f:
        nixpkgs.lib.genAttrs supportedSystems
        (system: f { pkgs = import nixpkgs { inherit system; }; });
    in {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell.override {
          # stdenv = pkgs.clangStdenv; # Clang instead of GCC
        } {
          packages = with pkgs; [
            alsa-lib # Lib
            alsa-utils # Lib
            # gcc # Compiler
            clang-tools # Clang CLIs
            cmake # Automation tool
            cmake-language-server # LSP
            cppcheck # Static analysis
            clang-uml # Generate UML from c(++) TEST it
            doctest # Testing framework
            doxygen # Documentation generator
            gdb # Debugger
            gnumake # Automation tool
            gtest # Testing framework
            lcov # Code coverage
            libsForQt5.full # Lib
            libpulseaudio # Lib
            lldb # Debug adapter
            pulseaudio # Lib
            rtmidi # Lib
            pkg-config # Find libraries
            valgrind # Debugging and profiling
          ];
          env = {
            CXX = "c++"; # Use Clang++ as the default C++ compiler
            CC = "clang"; # Use Clang as the default C compiler
            CXXFLAGS = ''
              -std=c++23 -Wall -Wextra -Wpedantic -Wshadow -Wconversion
            ''; # C++23 and more warnings
          };
        };
      });
    };
}
