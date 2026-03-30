#include "templates.h"
#include <stdio.h>
#include <string.h>

const char* get_dockerfile_template(LangType lang) {
    switch (lang) {
        case LANG_PYTHON:
            return "FROM python:3.11-slim\n"
                   "WORKDIR /app\n"
                   "COPY requirements.txt .\n"
                   "RUN pip install --no-cache-dir -r requirements.txt\n"
                   "COPY src/ ./src/\n"
                   "CMD [\"python\", \"src/main.py\"]\n";
        
        case LANG_NODE:
            return "FROM node:20-alpine\n"
                   "WORKDIR /app\n"
                   "COPY package*.json ./\n"
                   "RUN npm ci --only=production\n"
                   "COPY src/ ./src/\n"
                   "CMD [\"node\", \"src/index.js\"]\n";
        
        case LANG_GO:
            return "FROM golang:1.21-alpine AS builder\n"
                   "WORKDIR /app\n"
                   "COPY go.mod go.sum ./\n"
                   "RUN go mod download\n"
                   "COPY . .\n"
                   "RUN go build -o /app/main ./cmd/main.go\n"
                   "FROM alpine:latest\n"
                   "WORKDIR /root/\n"
                   "COPY --from=builder /app/main .\n"
                   "CMD [\"./main\"]\n";
        
        case LANG_RUST:
            return "FROM rust:1.75-alpine AS builder\n"
                   "WORKDIR /app\n"
                   "COPY Cargo.toml Cargo.lock ./\n"
                   "RUN mkdir src && echo \"fn main() {}\" > src/main.rs\n"
                   "RUN cargo build --release\n"
                   "RUN rm -rf src\n"
                   "COPY src ./src\n"
                   "RUN cargo build --release\n"
                   "FROM alpine:latest\n"
                   "COPY --from=builder /app/target/release/myapp /usr/local/bin/\n"
                   "CMD [\"myapp\"]\n";
        
        case LANG_C:
            return "FROM gcc:latest\n"
                   "WORKDIR /app\n"
                   "COPY src/ ./src/\n"
                   "COPY Makefile .\n"
                   "RUN make\n"
                   "CMD [\"./build/app\"]\n";
        
        default:
            return "# Dockerfile\nFROM alpine:latest\nWORKDIR /app\nCOPY . .\nCMD [\"sh\"]\n";
    }
}

const char* get_gitignore_template(LangType lang) {
    switch (lang) {
        case LANG_PYTHON:
            return "__pycache__/\n*.py[cod]\n*$py.class\n*.so\n.Python\nenv/\nvenv/\n.env\n.venv\n*.egg-info/\ndist/\nbuild/\n";
        
        case LANG_NODE:
            return "node_modules/\nnpm-debug.log\nyarn-error.log\n.env\n.DS_Store\ndist/\nbuild/\n";
        
        case LANG_GO:
            return "*.exe\n*.exe~\n*.dll\n*.so\n*.dylib\n*.test\n*.out\n/vendor/\n/bin/\n/dist/\n";
        
        case LANG_RUST:
            return "/target/\n**/*.rs.bk\n*.pdb\n*.dll\n*.exe\n*.rlib\n*.rmeta\nCargo.lock\n";
        
        case LANG_C:
            return "*.o\n*.exe\n*.out\n*.a\n*.so\n*.dll\n*.dylib\n/build/\n";
        
        default:
            return "build/\n*.log\n.env\n";
    }
}

// Helper function to build dynamic strings
static char* build_main_content(LangType lang, const char *project_name) {
    static char content[2048];
    
    switch (lang) {
        case LANG_PYTHON:
            snprintf(content, sizeof(content),
                "# src/main.py\n\n"
                "def main():\n"
                "    print(\"Hello from Python\")\n"
                "    print(\"Project: %s\")\n\n"
                "if __name__ == \"__main__\":\n"
                "    main()\n", project_name);
            return content;
        
        case LANG_NODE:
            snprintf(content, sizeof(content),
                "// src/index.js\n\n"
                "console.log('Hello from Node.js');\n"
                "console.log('Project: %s');\n\n"
                "const http = require('http');\n"
                "const server = http.createServer((req, res) => {\n"
                "    res.writeHead(200, {'Content-Type': 'text/plain'});\n"
                "    res.end('Hello from %s\\n');\n"
                "});\n"
                "server.listen(8080, () => {\n"
                "    console.log('Server running on port 8080');\n"
                "});\n", project_name, project_name);
            return content;
        
        case LANG_GO:
            snprintf(content, sizeof(content),
                "// cmd/main.go\n\n"
                "package main\n\n"
                "import (\n"
                "    \"fmt\"\n"
                "    \"net/http\"\n"
                ")\n\n"
                "func main() {\n"
                "    fmt.Println(\"Hello from Go\")\n"
                "    fmt.Println(\"Project: %s\")\n"
                "    http.HandleFunc(\"/\", func(w http.ResponseWriter, r *http.Request) {\n"
                "        fmt.Fprintf(w, \"Hello from %s\\n\")\n"
                "    })\n"
                "    http.ListenAndServe(\":8080\", nil)\n"
                "}\n", project_name, project_name);
            return content;
        
        case LANG_RUST:
            snprintf(content, sizeof(content),
                "// src/main.rs\n\n"
                "fn main() {\n"
                "    println!(\"Hello from Rust\");\n"
                "    println!(\"Project: %s\");\n"
                "}\n", project_name);
            return content;
        
        case LANG_C:
            snprintf(content, sizeof(content),
                "// src/main.c\n\n"
                "#include <stdio.h>\n\n"
                "int main() {\n"
                "    printf(\"Hello from C\\n\");\n"
                "    printf(\"Project: %s\\n\");\n"
                "    return 0;\n"
                "}\n", project_name);
            return content;
        
        default:
            snprintf(content, sizeof(content), "# Main file\nprint(\"Hello World\")\n");
            return content;
    }
}

const char* get_main_template(LangType lang, const char *project_name) {
    return build_main_content(lang, project_name);
}

const char* get_test_template(LangType lang) {
    switch (lang) {
        case LANG_PYTHON:
            return "# tests/test_main.py\n\n"
                   "import unittest\n\n"
                   "class TestMain(unittest.TestCase):\n"
                   "    def test_something(self):\n"
                   "        self.assertEqual(1, 1)\n\n"
                   "if __name__ == '__main__':\n"
                   "    unittest.main()\n";
        
        case LANG_NODE:
            return "// tests/test.js\n\n"
                   "const assert = require('assert');\n\n"
                   "describe('Main', () => {\n"
                   "    it('should work', () => {\n"
                   "        assert.equal(1, 1);\n"
                   "    });\n"
                   "});\n";
        
        default:
            return "# Test file\n# Add your tests here\n";
    }
}

const char* get_forge_yaml_template(const char *project_name, LangType lang) {
    static char yaml[1024];
    const char *lang_name = "";
    switch (lang) {
        case LANG_PYTHON: lang_name = "python"; break;
        case LANG_NODE: lang_name = "node"; break;
        case LANG_GO: lang_name = "go"; break;
        case LANG_RUST: lang_name = "rust"; break;
        case LANG_C: lang_name = "c"; break;
        default: lang_name = "custom";
    }
    snprintf(yaml, sizeof(yaml),
        "name: %s\n"
        "version: 1\n"
        "language: %s\n"
        "runtime: docker\n\n"
        "build:\n"
        "  command: # Build command\n\n"
        "run:\n"
        "  command: # Run command\n"
        "  port: 8080\n\n"
        "health_check:\n"
        "  path: /health\n"
        "  interval: 30s\n",
        project_name, lang_name);
    return yaml;
}

const char* get_readme_template(const char *project_name, LangType lang) {
    static char readme[2048];
    (void)lang; // Suppress unused parameter warning
    snprintf(readme, sizeof(readme),
        "# %s\n\n"
        "## Description\n\n"
        "Project initialized with Forge.\n\n"
        "## Development\n\n"
        "### Build\n"
        "```bash\n"
        "forge build\n"
        "```\n\n"
        "### Run\n"
        "```bash\n"
        "forge run\n"
        "```\n\n"
        "### Deploy\n"
        "```bash\n"
        "forge deploy\n"
        "```\n\n"
        "## License\n\n"
        "MIT\n",
        project_name);
    return readme;
}

const char* get_env_example_template(LangType lang) {
    (void)lang;
    return "# Environment variables\n"
           "# Copy this to .env and fill in your values\n\n"
           "DATABASE_URL=postgres://user:pass@localhost:5432/db\n"
           "API_KEY=your-api-key-here\n"
           "DEBUG=false\n";
}

const char* get_ci_template(CIType ci, LangType lang) {
    if (ci == CI_GITHUB) {
        switch (lang) {
            case LANG_PYTHON:
                return "name: CI\n\non: [push, pull_request]\n\njobs:\n"
                       "  test:\n"
                       "    runs-on: ubuntu-latest\n"
                       "    steps:\n"
                       "    - uses: actions/checkout@v3\n"
                       "    - name: Set up Python\n"
                       "      uses: actions/setup-python@v4\n"
                       "      with:\n"
                       "        python-version: '3.11'\n"
                       "    - name: Install dependencies\n"
                       "      run: pip install -r requirements.txt\n"
                       "    - name: Run tests\n"
                       "      run: python -m pytest tests/\n";
            default:
                return "name: CI\n\non: [push, pull_request]\n\njobs:\n"
                       "  test:\n"
                       "    runs-on: ubuntu-latest\n"
                       "    steps:\n"
                       "    - uses: actions/checkout@v3\n"
                       "    - name: Run tests\n"
                       "      run: echo \"Add your tests here\"\n";
        }
    }
    return "";
}