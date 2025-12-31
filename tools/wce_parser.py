#!/usr/bin/env python3
import argparse
import sys
import os
import re
from enum import Enum, auto

# --- Tokenizer ---

class TokenType(Enum):
    IDENTIFIER = auto()
    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    SEMICOLON = auto()
    STRING = auto()
    NUMBER = auto()
    COMMA = auto()
    UNKNOWN = auto()
    EOF = auto()

class Token:
    def __init__(self, type, value):
        self.type = type
        self.value = value
    
    def __repr__(self):
        return f"Token({self.type}, {self.value})"

class Tokenizer:
    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.length = len(text)
    
    def peek_char(self):
        if self.pos >= self.length:
            return None
        return self.text[self.pos]
    
    def consume_char(self):
        c = self.peek_char()
        if c:
            self.pos += 1
        return c
    
    def tokenize(self):
        tokens = []
        while self.pos < self.length:
            c = self.peek_char()
            
            if c.isspace():
                self.consume_char()
                continue
            
            if c == '/':
                # Handle comments
                if self.pos + 1 < self.length and self.text[self.pos+1] == '/':
                    while self.pos < self.length and self.peek_char() != '\n':
                        self.consume_char()
                    continue
            
            if c.isalpha() or c == '_':
                start = self.pos
                while self.pos < self.length and (self.peek_char().isalnum() or self.peek_char() == '_'):
                    self.consume_char()
                tokens.append(Token(TokenType.IDENTIFIER, self.text[start:self.pos]))
            
            elif c == '"':
                self.consume_char()
                start = self.pos
                while self.pos < self.length and self.peek_char() != '"':
                    if self.peek_char() == '\\':
                        self.consume_char()
                    self.consume_char()
                value = self.text[start:self.pos]
                self.consume_char() # Closing quote
                tokens.append(Token(TokenType.STRING, value))
            
            elif c.isdigit():
                start = self.pos
                while self.pos < self.length and self.peek_char().isdigit():
                    self.consume_char()
                tokens.append(Token(TokenType.NUMBER, self.text[start:self.pos]))
            
            elif c == '(':
                self.consume_char()
                tokens.append(Token(TokenType.LPAREN, '('))
            elif c == ')':
                self.consume_char()
                tokens.append(Token(TokenType.RPAREN, ')'))
            elif c == '{':
                self.consume_char()
                tokens.append(Token(TokenType.LBRACE, '{'))
            elif c == '}':
                self.consume_char()
                tokens.append(Token(TokenType.RBRACE, '}'))
            elif c == ';':
                self.consume_char()
                tokens.append(Token(TokenType.SEMICOLON, ';'))
            elif c == ',':
                self.consume_char()
                tokens.append(Token(TokenType.COMMA, ','))
            else:
                self.consume_char()
                # Ignore unknown chars or handle them
        
        tokens.append(Token(TokenType.EOF, ''))
        return TokenStream(tokens)

class TokenStream:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
    
    def peek(self, type=None):
        if self.pos >= len(self.tokens):
            return None
        token = self.tokens[self.pos]
        if type and token.type != type:
            return None
        return token
    
    def consume(self):
        if self.pos < len(self.tokens):
            t = self.tokens[self.pos]
            self.pos += 1
            return t
        return None
    
    def match(self, type, value_prefix=None):
        token = self.peek()
        if not token: return False
        if token.type != type: return False
        if value_prefix and not token.value.startswith(value_prefix): return False
        return True
    
    def expect(self, type):
        token = self.consume()
        if not token or token.type != type:
            raise Exception(f"Expected {type}, got {token}")
        return token

# --- AST ---

class AstNode:
    def __init__(self, node_type, name=None):
        self.type = node_type
        self.name = name
        self.children = []
        self.args = []
    
    def add_child(self, child):
        self.children.append(child)

# --- Parser ---

class WceParser:
    def __init__(self):
        self.context_stack = []
        self.root = AstNode("ROOT")
        self.context_stack.append(self.root)
        
        self.style_content = ""
        self.html_content = ""
        self.script_content = ""

    def parse_file(self, filepath):
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()

        # Extract blocks (Legacy support)
        self.style_content = self._extract_block(content, "WCE_STYLE_BEGIN", "WCE_STYLE_END")
        self.html_content = self._extract_block(content, "WCE_HTML_BEGIN", "WCE_HTML_END")
        self.script_content = self._extract_block(content, "WCE_SCRIPT_BEGIN", "WCE_SCRIPT_END")
        
        # Parse UI Block (New Syntax)
        ui_content = self._extract_block(content, "WCE_UI_BEGIN", "WCE_UI_END")
        if ui_content:
            self._parse_ui_code(ui_content)
            self.style_content += self._get_default_ui_css()

    def _extract_block(self, content, start_marker, end_marker):
        start = content.find(start_marker)
        if start == -1: return ""
        start += len(start_marker)
        end = content.find(end_marker, start)
        if end == -1: return ""
        return content[start:end].strip()

    def _get_default_ui_css(self):
        return """
.wce-row { display: flex; flex-direction: row; gap: 10px; margin-bottom: 10px; }
.wce-col { display: flex; flex-direction: column; flex: 1; }
.wce-card { background: #fff; padding: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
.wce-container { padding: 20px; }
"""

    def _parse_ui_code(self, code):
        tokenizer = Tokenizer(code)
        tokens = tokenizer.tokenize()
        
        while not tokens.match(TokenType.EOF):
            self.parse_statement(tokens)
        
        # Generate HTML from AST
        self.html_content += self._generate_html(self.root)

    def parse_statement(self, tokens):
        if tokens.match(TokenType.IDENTIFIER, 'wce_'):
            return self.parse_function_scope(tokens)
        else:
            tokens.consume() # Skip unknown
            return None

    def parse_function_scope(self, tokens):
        """解析 wce_row({ ... }) 这样的函数作用域"""
        if not tokens.match(TokenType.IDENTIFIER, 'wce_'):
            return None
        
        func_name = tokens.consume().value
        tokens.expect(TokenType.LPAREN)
        
        # 检查是否为新语法
        if tokens.peek(TokenType.LBRACE):
            tokens.consume()  # 吃掉 {
            
            # 创建容器节点
            node_type = func_name.replace('wce_', '')
            if node_type == 'column':
                node_type = 'col'
            node = AstNode(node_type=node_type, name=func_name)
            
            current = self.context_stack[-1]
            current.add_child(node)
            self.context_stack.append(node)
            
            # 解析大括号内的所有语句
            while not tokens.peek(TokenType.RBRACE) and not tokens.peek(TokenType.EOF):
                self.parse_statement(tokens)
                
                # 可选的分号
                if tokens.peek(TokenType.SEMICOLON):
                    tokens.consume()
            
            tokens.expect(TokenType.RBRACE)
            tokens.expect(TokenType.RPAREN)
            if tokens.peek(TokenType.SEMICOLON):
                tokens.consume()
            
            self.context_stack.pop()
            return node
        else:
            # 回退到旧语法解析 (Leaf node or legacy)
            return self.parse_legacy_component(tokens, func_name)

    def parse_legacy_component(self, tokens, func_name):
        node_type = func_name.replace('wce_', '')
        node = AstNode(node_type=node_type, name=func_name)
        
        # Parse args
        args = []
        while not tokens.peek(TokenType.RPAREN) and not tokens.peek(TokenType.EOF):
            t = tokens.consume()
            args.append(t.value)
        
        tokens.expect(TokenType.RPAREN)
        if tokens.peek(TokenType.SEMICOLON):
            tokens.consume()
            
        node.args = args
        
        current = self.context_stack[-1]
        current.add_child(node)
        return node

    def _generate_html(self, node):
        html = ""
        if node.type == "ROOT":
            for child in node.children:
                html += self._generate_html(child)
        elif node.type in ["row", "col", "card", "container", "panel"]:
            html += f'<div class="wce-{node.type}">\n'
            for child in node.children:
                html += self._generate_html(child)
            html += '</div>\n'
        else:
            # Leaf nodes
            args_str = ", ".join(node.args)
            html += f'<!-- {node.name}({args_str}) -->\n'
            if node.type == "text":
                html += f'<span>{args_str}</span>\n'
            elif node.type == "button":
                html += f'<button onclick="{node.args[1] if len(node.args)>1 else ""}">{node.args[0] if len(node.args)>0 else "Button"}</button>\n'
        return html

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Input .wce file")
    args = parser.parse_args()
    
    if os.path.exists(args.input):
        p = WceParser()
        p.parse_file(args.input)
        print(p.html_content)
        
        with open("webcee_output.html", "w") as f:
            f.write(p.html_content)
