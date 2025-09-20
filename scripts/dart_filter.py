#!/usr/bin/env python3
"""
Dart to C++ filter for Doxygen
Converts Dart syntax to C++-like syntax that Doxygen can understand
"""

import sys
import re

def dart_to_cpp_filter(content):
    """Convert Dart code to C++-like syntax for Doxygen parsing"""
    
    # Convert Dart documentation comments to Doxygen format
    content = re.sub(r'///', r'///', content)
    
    # Convert class declarations
    content = re.sub(r'class\s+(\w+)\s+extends\s+(\w+)', r'class \1 : public \2', content)
    content = re.sub(r'class\s+(\w+)\s+implements\s+(\w+)', r'class \1 : public \2', content)
    content = re.sub(r'class\s+(\w+)\s+with\s+(\w+)', r'class \1 : public \2', content)
    content = re.sub(r'abstract class\s+(\w+)', r'class \1', content)
    
    # Convert method declarations
    content = re.sub(r'(\w+)\s*\(([^)]*)\)\s*\{', r'\1(\2) {', content)
    content = re.sub(r'(\w+)\s*\(([^)]*)\)\s*async\s*\{', r'Future<void> \1(\2) {', content)
    content = re.sub(r'Future<(\w+)>\s+(\w+)\s*\(([^)]*)\)', r'Future<\1> \2(\3)', content)
    
    # Convert variable declarations
    content = re.sub(r'final\s+(\w+)\s+(\w+)', r'\1 \2', content)
    content = re.sub(r'var\s+(\w+)', r'auto \1', content)
    content = re.sub(r'const\s+(\w+)\s+(\w+)', r'const \1 \2', content)
    
    # Convert constructors
    content = re.sub(r'(\w+)\({[^}]*}\)', r'\1()', content)
    
    # Convert factory constructors
    content = re.sub(r'factory\s+(\w+)', r'static \1', content)
    
    # Convert static methods
    content = re.sub(r'static\s+(\w+)\s+(\w+)\s*\(([^)]*)\)', r'static \1 \2(\3)', content)
    
    # Convert override annotation
    content = re.sub(r'@override', r'virtual', content)
    
    # Convert getter/setter
    content = re.sub(r'(\w+)\s+get\s+(\w+)\s*=>', r'\1 get\2() { return', content)
    content = re.sub(r'set\s+(\w+)\s*\(([^)]*)\)\s*=>', r'void set\1(\2) {', content)
    
    # Convert enum
    content = re.sub(r'enum\s+(\w+)\s*\{', r'enum class \1 {', content)
    
    # Convert typedef
    content = re.sub(r'typedef\s+(\w+)\s*=\s*([^;]+);', r'typedef \2 \1;', content)
    
    return content

def main():
    if len(sys.argv) != 2:
        print("Usage: dart_filter.py <dart_file>", file=sys.stderr)
        sys.exit(1)
    
    try:
        with open(sys.argv[1], 'r', encoding='utf-8') as f:
            content = f.read()
        
        filtered_content = dart_to_cpp_filter(content)
        print(filtered_content)
        
    except Exception as e:
        print(f"Error processing file {sys.argv[1]}: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()