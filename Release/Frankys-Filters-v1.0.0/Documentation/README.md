# Franky's Filters

A professional-grade multi-mode audio filter plugin built with JUCE framework. Franky's Filters offers precise frequency control with multiple filter types and variable slopes, perfect for music production, mixing, and sound design.

## Features

### Filter Types
- **Low-pass Filter**: Removes frequencies above the cutoff point
- **High-pass Filter**: Removes frequencies below the cutoff point  
- **Band-pass Filter**: Allows only frequencies around the cutoff point to pass

### Filter Slopes
- **6 dB/octave**: Gentle, musical filtering with minimal phase shift
- **12 dB/octave**: Standard slope for most applications
- **24 dB/octave**: Steep filtering for dramatic effects

### Controls
- **Cutoff Frequency**: 20 Hz - 20 kHz with logarithmic scaling
- **Resonance**: 0.1 - 5.0 Q factor for filter emphasis
- **Gain**: -24 dB to +12 dB post-filter gain compensation
- **Real-time parameter smoothing** to prevent audio artifacts
- **Preset system** for saving and recalling your favorite settings

### Technical Specifications
- **Sample Rate Support**: Up to 192 kHz
- **Bit Depth**: 32-bit floating point processing
- **Latency**: Zero latency processing
- **CPU Usage**: Optimized for real-time performance
- **Channel Support**: Stereo (can be used as mono)

## Installation

### System Requirements
- **macOS**: 10.13 or later (Intel and Apple Silicon supported)
- **Audio Unit (AU)** or **VST3** compatible host application
- **Memory**: Minimal RAM usage (< 10 MB)

### Download and Install

#### Option 1: Download Pre-built Binaries
1. Go to the [Releases](../../releases) page
2. Download the latest version for your platform
3. Extract the downloaded file

#### Option 2: Build from Source
```bash
# Clone the repository
git clone https://github.com/yourusername/frankys-filters.git
cd frankys-filters/MultiFilter

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the plugin
make -j4
```

### Installation Paths

After downloading or building, copy the plugin files to the appropriate directories:

#### Audio Unit (AU)
```bash
# Copy to AU plugins directory
cp -r "build/MyAwesomePlugin_artefacts/AU/Franky's Filters.component" ~/Library/Audio/Plug-Ins/Components/
```

#### VST3
```bash
# Copy to VST3 plugins directory  
cp -r "build/MyAwesomePlugin_artefacts/VST3/Franky's Filters.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

### Verify Installation
1. Restart your DAW or host application
2. Scan for new plugins (if required by your DAW)
3. Look for "Franky's Filters" in your plugin list
4. The plugin should appear under "Awesome Audio Co" manufacturer

## Usage

### Basic Operation
1. **Load the plugin** on an audio track or bus
2. **Select filter type** using the Low-pass, High-pass, or Band-pass buttons
3. **Choose filter slope** (6dB, 12dB, or 24dB) for the desired steepness
4. **Adjust cutoff frequency** to set the filter's center point
5. **Set resonance** to add emphasis at the cutoff frequency
6. **Use gain control** to compensate for level changes

### Tips for Best Results
- **Start with 12dB slope** for general use
- **Use high resonance sparingly** to avoid unwanted resonant peaks
- **Low-pass filters** work great for removing harshness from bright sources
- **High-pass filters** are perfect for cleaning up low-end mud
- **Band-pass filters** can isolate specific frequency ranges

### Preset Management
- Use the preset system to save your favorite filter settings
- Presets store all parameters including filter type and slope
- Great for quickly switching between different filter characters

## Supported DAWs

Franky's Filters has been tested with:
- **Logic Pro X/Logic Pro**
- **Pro Tools**
- **Ableton Live**
- **Reaper**
- **Cubase/Nuendo**
- **Studio One**
- **Garage Band**
- **MainStage**

## Troubleshooting

### Plugin Not Appearing
1. Verify the plugin is in the correct directory
2. Restart your DAW completely
3. Trigger a plugin rescan in your DAW settings
4. Check that your DAW supports AU or VST3 format

### Audio Issues
- If you hear crackling, try increasing your audio buffer size
- For CPU usage issues, close other applications
- Check your sample rate matches between DAW and plugin

### Plugin Validation
You can validate the plugin using Apple's auval tool:
```bash
auval -v aufx AWSM Awsm
```

## Technical Details

### Architecture
- Built with **JUCE 7.x** framework
- Uses **TPT (Topology Preserving Transform)** filters for superior sound quality
- Cascaded filter design for higher-order slopes
- Parameter smoothing prevents zipper noise
- Optimized for both Intel and Apple Silicon processors

### Parameters
- **Cutoff**: Exponential scaling from 20Hz to 20kHz
- **Resonance**: Linear scaling from 0.1 to 5.0 Q
- **Gain**: Linear scaling from -24dB to +12dB
- **Filter Type**: Choice parameter (Low/High/Band-pass)
- **Slope**: Choice parameter (6/12/24 dB/octave)

## Development

### Building from Source
Requirements:
- CMake 3.22 or later
- JUCE framework installed in `/Applications/JUCE`
- Xcode Command Line Tools

### Contributing
We welcome contributions! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

- **Developer**: Franky Redente
- **Framework**: JUCE by Roli Ltd.
- **Company**: Awesome Audio Co
- **Built with**: Agentic AI assistance from [Claude Code](https://claude.ai/code) by Anthropic

**Note**: This entire project was developed using agentic AI in Claude Code, demonstrating the power of AI-assisted software development for creating professional-grade audio plugins.

## Support

For support, feature requests, or bug reports:
- **GitHub Issues**: [Create an issue](../../issues)
- **Email**: support@awesomeaudio.co

---

**Version**: 1.0.0  
**Last Updated**: September 2025  
**Compatibility**: macOS 10.13+, AU/VST3 hosts