<div align="center">

# 🎨 PineLock Brand Guidelines
## Visual Identity & Design System

**Version:** 1.0  
**Last Updated:** November 21, 2025

---

</div>

## 🌲 Logo & Brand Identity

### Logo Concept

The PineLock logo combines two key elements:
- **🌲 Pine Tree (Choinka)** - Represents "Pine" in PineLock, symbolizing nature, strength, and the cabin/forest environment
- **🔑 Keyhole** - Represents "Lock", symbolizing security and access control

**Color Scheme:**
- **Primary:** Forest Green `#1e5945` (Trust, Security, Nature)
- **Secondary:** Dark Green `#2d6f51` (Depth, Reliability)
- **Accent:** White/Cream for keyhole (Clarity, Openness)

---

## 🎨 Design System

### Color Palette

```
Primary Colors:
┌─────────────────────────────────────┐
│ Forest Green  #1e5945  ███████████  │
│ Dark Green    #2d6f51  ███████████  │
│ Pine Green    #3a8660  ███████████  │
└─────────────────────────────────────┘

Accent Colors:
┌─────────────────────────────────────┐
│ White         #ffffff  ███████████  │
│ Cream         #f5f5dc  ███████████  │
│ Light Gray    #e8e8e8  ███████████  │
└─────────────────────────────────────┘

Status Colors:
┌─────────────────────────────────────┐
│ Success       #28a745  ███████████  │
│ Warning       #ffc107  ███████████  │
│ Error         #dc3545  ███████████  │
│ Info          #17a2b8  ███████████  │
└─────────────────────────────────────┘
```

### Typography

**Markdown Headings:**
- H1: `# 🌲 Title` - Main branding
- H2: `## 📚 Section` - Major sections  
- H3: `### ⚡ Subsection` - Components

**Code Blocks:**
- Use syntax highlighting
- Include language identifier
- Add comments for clarity

---

## 🎯 Icon System

### Core Icons

| Category | Icon | Usage |
|----------|------|-------|
| **Security** | 🔐 🔒 🔓 🔑 | Lock states, keys |
| **Hardware** | 🔧 🛠️ ⚡ 🔌 | Tools, setup, power |
| **Network** | 📡 🌐 🔄 📶 | WiFi, MQTT, sync |
| **Status** | ✅ ❌ ⚠️ ℹ️ | Success, error, warning |
| **Data** | 📊 📈 📉 💾 | Analytics, storage |
| **Docs** | 📖 📝 📋 📄 | Documentation |
| **Time** | ⏰ ⏱️ 🕐 ⏳ | RTC, timing |
| **Input** | 🎹 🔢 📇 📟 | Keypad, PIN, RFID |

### Emoji Usage Guidelines

✅ **Do:**
- Use consistently across documentation
- Match emoji to content meaning
- Place at start of headings for scanability

❌ **Don't:**
- Overuse in body text
- Mix similar icons inconsistently
- Use decorative emojis without purpose

---

## 📐 Layout Guidelines

### Document Structure

```markdown
<div align="center">

# 🌲 Document Title
## Subtitle or Tagline

**Key Info** | **More Info**

[![Badge](link)]()

---

</div>

## 📚 Content Section

Body text with proper formatting...
```

### Badges

**Standard Badges:**
```markdown
[![Platform](https://img.shields.io/badge/platform-ESP32--C3-blue.svg)]()
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()
[![Status](https://img.shields.io/badge/status-Beta-yellow.svg)]()
```

**Custom Badges:**
- Use shields.io for consistency
- Match colors to brand palette
- Keep text concise

---

## 📝 Writing Style

### Tone & Voice

**Characteristics:**
- **Professional** - Technical accuracy, clear explanations
- **Friendly** - Approachable language, helpful tone
- **Confident** - Assert capabilities, acknowledge limitations
- **Concise** - Respect reader's time, get to the point

**Example:**
```
✅ Good: "Configure WiFi credentials in config.h before deployment"
❌ Avoid: "You might want to maybe set up your WiFi or something in the config file I guess"
```

### Technical Writing

**Code Examples:**
- Always include comments
- Show complete, working examples
- Explain parameters
- Include expected output

**Warnings & Notes:**
```markdown
> ⚠️ **Warning**: Critical safety or security information
> 💡 **Tip**: Helpful suggestion or best practice  
> 📝 **Note**: Additional context or clarification
> 🔐 **Security**: Security-related information
```

---

## 🖼️ ASCII Art & Diagrams

### System Diagram Template

```
┌──────────────────────────────────────────────────────────┐
│                     Component Name                        │
│  ┌────────────────────────────────────────────────────┐  │
│  │  Detail 1                                          │  │
│  │  Detail 2                                          │  │
│  │  Detail 3                                          │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

### Connection Diagrams

```
Device A ──→ Device B
         ←── 
         
┌─────────┐    I2C     ┌─────────┐
│ ESP32   │◄──────────►│ PCF8574 │
└─────────┘            └─────────┘
```

---

## 📊 Tables & Data Presentation

### Feature Tables

```markdown
<table>
<tr>
<td width="50%">

### Column 1 Title
- Feature 1
- Feature 2

</td>
<td width="50%">

### Column 2 Title  
- Feature 3
- Feature 4

</td>
</tr>
</table>
```

### Comparison Tables

| Feature | Before | After | Impact |
|---------|--------|-------|--------|
| Item 1 | ❌ | ✅ | 🟢 High |
| Item 2 | ⚠️ | ✅ | 🟡 Medium |

---

## 🎬 Animation & Effects

### Progress Indicators

```
[████████████░░░░░░░░] 60%
[====================] Complete!
```

### Status Boxes

```
┌─────────────────────────────────┐
│ ✅ SYSTEM READY                 │
│ Status: Operational             │
│ Uptime: 48 days                 │
└─────────────────────────────────┘
```

---

## 📱 Responsive Design

### Mobile-Friendly Tables

Use HTML tables with width attributes:
```html
<table>
<tr>
<th width="30%">Column 1</th>
<th width="70%">Column 2</th>
</tr>
</table>
```

### Collapsible Sections

```markdown
<details>
<summary>📋 <b>Click to Expand</b></summary>

Hidden content here...

</details>
```

---

## 🌐 Multilingual Considerations

### Primary Language
- **English** - Main documentation
- **Polish** - Alternative/internal docs

### Technical Terms
- Keep technical terms in English
- Provide translations in parentheses when helpful
- Maintain consistency across documents

---

## ✅ Quality Checklist

Before publishing documentation:

- [ ] Logo/branding in header
- [ ] Consistent emoji usage
- [ ] All code blocks have syntax highlighting
- [ ] Tables are properly formatted
- [ ] Links are working
- [ ] Badges are current
- [ ] Spelling checked
- [ ] Grammar reviewed
- [ ] Technical accuracy verified
- [ ] Navigation links included

---

## 📚 Template Library

### README Template
See: [README.md](README.md)

### Setup Guide Template  
See: [SETUP.md](SETUP.md)

### Deployment Guide Template
See: [DEPLOYMENT.md](DEPLOYMENT.md)

### Code Review Template
See: [CODE_REVIEW_REPORT.md](CODE_REVIEW_REPORT.md)

---

<div align="center">

## 🌲 PineLock Branding

**Consistency. Clarity. Quality.**

*Secure your space. Protect what matters.*

---

[![GitHub](https://img.shields.io/badge/GitHub-PineLock-181717?logo=github)](https://github.com/kmush12/PineLock)

**Version 1.0** | Last Updated: November 21, 2025

</div>
