# Contributing to PowderSense

**Thank you for your interest in contributing to PowderSense!**

This document provides guidelines for contributing to the project. By participating, you agree to maintain a respectful and collaborative environment.

---

## 🌟 Ways to Contribute

### 1. Report Bugs

Found a bug? Help us fix it!

**Before reporting**:
- Search existing issues to avoid duplicates
- Test with the latest version
- Verify it's reproducible

**When reporting, include**:
- Clear description of the issue
- Steps to reproduce
- Expected vs actual behavior
- Hardware variant (touch/RGB)
- Firmware version
- Serial monitor output (if applicable)
- Photos/screenshots (if relevant)

**Template**:
```markdown
**Description**: Brief summary of the bug

**Steps to Reproduce**:
1. Step one
2. Step two
3. Step three

**Expected Behavior**: What should happen

**Actual Behavior**: What actually happens

**Environment**:
- Board: ESP32-C6 Touch / RGB
- Firmware: v1.0.0
- Browser: Chrome 120 / Firefox 121

**Additional Context**: Any other relevant information
```

### 2. Suggest Features

Have an idea for improvement?

**Before suggesting**:
- Check existing feature requests
- Consider if it fits project scope
- Think about implementation complexity

**When suggesting, include**:
- Use case description
- Proposed solution
- Alternative approaches
- Potential challenges
- Willingness to implement

### 3. Improve Documentation

Documentation is crucial for adoption!

**Areas needing help**:
- Tutorials and guides
- Code comments
- API documentation
- Translation to other languages
- Video tutorials
- Application notes

**Documentation standards**:
- Clear and concise language
- Step-by-step instructions
- Screenshots where helpful
- Code examples that work
- Markdown formatting

### 4. Contribute Code

Code contributions are welcome!

**Types of contributions**:
- Bug fixes
- New features
- Performance improvements
- Code refactoring
- Test coverage
- Library updates

**Before coding**:
- Discuss major changes in an issue first
- Check if someone else is working on it
- Ensure it aligns with project goals

### 5. Hardware Variants

Created a hardware adaptation?

**Share**:
- Schematics and PCB layouts
- Bill of materials
- Assembly instructions
- Photos of completed build
- Firmware modifications (if any)

**Examples**:
- Different display sizes
- Alternative sensors
- Custom enclosures
- Integration with other devices

### 6. Application Notes

Using PowderSense in a unique way?

**Document**:
- Your specific use case
- Hardware setup
- Configuration settings
- Calibration procedure
- Results and lessons learned

---

## 🔧 Development Setup

### Prerequisites

1. **Install PlatformIO**
   - VS Code + PlatformIO extension (recommended)
   - Or PlatformIO Core CLI

2. **Clone Repository**
   ```bash
   git clone https://github.com/me-processware/powdersense.git
   cd powdersense
   ```

3. **Install Dependencies**
   ```bash
   pio pkg install
   ```

4. **Build Project**
   ```bash
   pio run -e esp32c6_touch
   ```

### Development Environment

**Recommended Tools**:
- Visual Studio Code
- PlatformIO IDE extension
- Serial Monitor (built-in or external)
- Git for version control

**Optional Tools**:
- KiCad (for PCB design)
- FreeCAD/Fusion 360 (for enclosure design)
- Wireshark (for network debugging)

---

## 📝 Code Guidelines

### C++ Style

**General Principles**:
- Write clear, readable code
- Comment why, not what
- Keep functions focused and small
- Avoid premature optimization

**Naming Conventions**:
```cpp
// Variables: camelCase
int sensorValue = 0;
float currentDepth = 0.0;

// Constants: UPPER_SNAKE_CASE
const int MAX_READINGS = 100;
const float PI_VALUE = 3.14159;

// Functions: camelCase
void updateDisplay() { }
float calculateAverage() { }

// Classes: PascalCase
class MeasurementManager { };
class WebServerHandler { };

// Private members: leading underscore
class MyClass {
private:
  int _privateValue;
};
```

**Formatting**:
```cpp
// Indentation: 2 spaces (no tabs)
void myFunction() {
  if (condition) {
    doSomething();
  } else {
    doSomethingElse();
  }
}

// Braces: K&R style
if (condition) {
  // code
}

// Line length: 100 characters max (flexible)
```

**Comments**:
```cpp
// Good: Explains why
// Use moving average to reduce ADC noise from power supply
float average = calculateMovingAverage(readings);

// Bad: Explains what (obvious from code)
// Calculate average
float average = calculateMovingAverage(readings);

// Good: Document complex algorithms
/**
 * Implements exponential moving average filter
 * Formula: EMA = α * current + (1-α) * previous
 * α = 0.2 provides good balance between responsiveness and smoothing
 */
float calculateEMA(float current, float previous) {
  const float alpha = 0.2;
  return alpha * current + (1.0 - alpha) * previous;
}
```

### Arduino/ESP-IDF Specifics

**Avoid blocking delays**:
```cpp
// Bad
delay(1000);

// Good
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 1000;

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = now;
    // Do periodic task
  }
}
```

**Memory management**:
```cpp
// Prefer stack allocation
char buffer[64];

// Use heap sparingly
char* dynamicBuffer = new char[size];
// ... use buffer ...
delete[] dynamicBuffer;

// Or use smart pointers (C++11)
std::unique_ptr<char[]> smartBuffer(new char[size]);
```

**String handling**:
```cpp
// Prefer String class for simplicity
String message = "Hello " + userName;

// Use char arrays for performance-critical code
char message[64];
snprintf(message, sizeof(message), "Hello %s", userName);
```

### Web Interface (HTML/CSS/JS)

**HTML**:
- Semantic markup
- Accessibility attributes
- Mobile-responsive design

**CSS**:
- Use CSS variables for theming
- Mobile-first approach
- Minimize inline styles

**JavaScript**:
- Modern ES6+ syntax
- Avoid jQuery (use vanilla JS)
- Handle errors gracefully
- Comment complex logic

---

## 🧪 Testing

### Hardware Testing

**Before submitting**:
- [ ] Test on actual hardware (not just simulation)
- [ ] Test both board variants (touch and RGB)
- [ ] Verify calibration accuracy
- [ ] Check web interface on multiple browsers
- [ ] Test WiFi connectivity (AP mode and client mode)
- [ ] Verify OTA updates work
- [ ] Check serial monitor for errors

**Test Scenarios**:
1. **Fresh install**: Flash firmware to blank device
2. **Upgrade**: Update from previous version
3. **WiFi**: Connect to different networks
4. **Measurement**: Verify accuracy with known references
5. **Web UI**: Test all buttons and inputs
6. **Edge cases**: Power loss, network drop, invalid input

### Software Testing

**Unit Tests** (future):
- Test individual functions
- Mock hardware interfaces
- Verify calculations

**Integration Tests**:
- Test component interactions
- Verify WebSocket communication
- Check data persistence

---

## 🔀 Pull Request Process

### 1. Fork and Branch

```bash
# Fork repository on GitHub
# Clone your fork
git clone https://github.com/YOUR_USERNAME/powdersense.git

# Create feature branch
git checkout -b feature/your-feature-name

# Or bugfix branch
git checkout -b bugfix/issue-123
```

### 2. Make Changes

- Write code following style guidelines
- Add comments where needed
- Update documentation
- Test thoroughly

### 3. Commit

**Commit Message Format**:
```
<type>: <short summary>

<optional detailed description>

<optional footer>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code formatting (no logic change)
- `refactor`: Code restructuring
- `perf`: Performance improvement
- `test`: Adding tests
- `chore`: Maintenance tasks

**Examples**:
```bash
git commit -m "feat: add Bluetooth connectivity"

git commit -m "fix: correct ADC calibration formula

Previous formula had incorrect scaling factor.
Now matches datasheet specification."

git commit -m "docs: update calibration guide with photos"
```

### 4. Push and Create PR

```bash
# Push to your fork
git push origin feature/your-feature-name

# Create pull request on GitHub
# Fill out PR template
```

### 5. PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring
- [ ] Performance improvement

## Testing
- [ ] Tested on ESP32-C6 Touch variant
- [ ] Tested on ESP32-C6 RGB variant
- [ ] Web interface tested
- [ ] Documentation updated

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings generated
- [ ] Tested on actual hardware

## Screenshots (if applicable)
[Add screenshots here]

## Additional Notes
[Any other relevant information]
```

### 6. Review Process

**What to expect**:
- Maintainer will review within 1-2 weeks
- May request changes or clarifications
- Discussion and iteration are normal
- Be patient and respectful

**After approval**:
- PR will be merged
- You'll be credited in release notes
- Thanks for contributing! 🎉

---

## 📋 Coding Checklist

Before submitting code, verify:

### Functionality
- [ ] Code compiles without errors
- [ ] Code runs on actual hardware
- [ ] Feature works as intended
- [ ] No regression (existing features still work)
- [ ] Edge cases handled

### Code Quality
- [ ] Follows style guidelines
- [ ] No compiler warnings
- [ ] No memory leaks
- [ ] Efficient (no unnecessary loops/delays)
- [ ] Readable and maintainable

### Documentation
- [ ] Code comments added
- [ ] README updated (if needed)
- [ ] API docs updated (if applicable)
- [ ] CHANGELOG.md entry added

### Testing
- [ ] Manual testing completed
- [ ] Multiple scenarios tested
- [ ] Error handling verified
- [ ] Serial output checked

---

## 🐛 Bug Fix Workflow

### 1. Reproduce Bug

- Verify bug exists
- Identify conditions that trigger it
- Document steps to reproduce

### 2. Locate Issue

- Use serial debugging
- Add temporary log statements
- Isolate problematic code section

### 3. Fix and Test

- Make minimal changes
- Test fix thoroughly
- Verify no side effects

### 4. Submit PR

- Reference issue number
- Explain root cause
- Describe fix approach

---

## ✨ Feature Development Workflow

### 1. Propose Feature

- Open GitHub issue
- Describe use case
- Discuss implementation
- Get maintainer feedback

### 2. Design

- Plan architecture
- Consider edge cases
- Think about backwards compatibility
- Document design decisions

### 3. Implement

- Start with minimal viable version
- Add tests
- Update documentation
- Iterate based on feedback

### 4. Submit PR

- Reference feature request issue
- Provide usage examples
- Include screenshots/videos
- Explain design choices

---

## 📚 Documentation Contributions

### Types of Documentation

**Code Documentation**:
- Inline comments
- Function/class headers
- API documentation

**User Documentation**:
- Setup guides
- Tutorials
- Troubleshooting
- FAQs

**Developer Documentation**:
- Architecture overview
- Contributing guide
- API reference
- Development setup

### Documentation Standards

**Structure**:
- Clear headings
- Logical flow
- Table of contents (for long docs)
- Cross-references

**Content**:
- Accurate and up-to-date
- Clear and concise
- Examples included
- Screenshots where helpful

**Formatting**:
- Markdown syntax
- Code blocks with syntax highlighting
- Tables for structured data
- Lists for steps/items

---

## 🌍 Translation

Help make PowderSense accessible to non-English speakers!

**What to translate**:
- Documentation (README, guides)
- Web interface strings
- Error messages
- Comments (optional)

**How to contribute**:
1. Create `docs/[language_code]/` directory
2. Translate markdown files
3. Maintain same structure as English version
4. Submit PR with translations

**Languages needed**:
- Spanish
- German
- French
- Chinese
- Japanese
- Others welcome!

---

## 🤝 Community Guidelines

### Code of Conduct

**Be respectful**:
- Treat everyone with respect
- Welcome newcomers
- Be patient with questions
- Assume good intentions

**Be constructive**:
- Provide helpful feedback
- Suggest improvements
- Explain reasoning
- Offer alternatives

**Be collaborative**:
- Share knowledge
- Help others
- Give credit
- Celebrate contributions

### Communication Channels

**GitHub Issues**: Bug reports, feature requests  
**GitHub Discussions**: Questions, ideas, general chat  
**Email**: Private inquiries

### Response Times

**Maintainer availability**:
- Issues reviewed: 1-2 weeks
- PRs reviewed: 1-2 weeks
- Questions answered: Best effort

**Please be patient** - this is an open source project maintained by volunteers.

---

## 🏆 Recognition

### Contributors

All contributors are recognized in:
- CONTRIBUTORS.md file
- Release notes
- GitHub contributors page

### Hall of Fame

Outstanding contributions may be featured:
- Project README
- Project website
- Social media mentions

---

## 📞 Questions?

**Not sure where to start?**
- Check existing issues for "good first issue" label
- Ask in GitHub Discussions

**Need help with contribution?**
- Ask questions in your PR
- Request review from maintainers
- Don't hesitate to ask for clarification

---

## 📄 License

By contributing, you agree that your contributions will be licensed under:

- **Hardware**: CERN-OHL-W
- **Software**: MIT License
- **Documentation**: CC BY-SA 4.0

You retain copyright of your contributions but grant the project rights to use them under these licenses.

---

**Thank you for contributing to PowderSense!**

Your contributions help make this project better for everyone. Whether you're fixing a typo, reporting a bug, or adding a major feature, every contribution is valued.

*Happy coding!* 🚀

---

*Maintained by Processware*  
*Last updated: November 2025*
