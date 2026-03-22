#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
WORKFLOW_FILE="$PROJECT_DIR/.github/workflows/build-recalbox.yml"

echo "=========================================="
echo "  Recalbox Workflow Validation Script"
echo "=========================================="
echo ""

check_yaml_syntax() {
    echo "✓ Checking YAML syntax..."
    if command -v ruby &> /dev/null; then
        if ruby -ryaml -e "YAML.load_file('$WORKFLOW_FILE')" 2>/dev/null; then
            echo "  YAML syntax: OK"
        else
            ruby -ryaml -e "YAML.load_file('$WORKFLOW_FILE')" 2>&1
            exit 1
        fi
    elif command -v python3 &> /dev/null; then
        if python3 -c "import yaml" 2>/dev/null; then
            python3 -c "import yaml; yaml.safe_load(open('$WORKFLOW_FILE'))" 2>/dev/null && echo "  YAML syntax: OK" || { echo "  YAML syntax: FAILED"; exit 1; }
        else
            echo "  Warning: No YAML validator available, skipping syntax check"
        fi
    elif command -v yamllint &> /dev/null; then
        yamllint "$WORKFLOW_FILE" && echo "  YAML syntax: OK" || { echo "  YAML syntax: FAILED"; exit 1; }
    else
        echo "  Warning: No YAML validator found, skipping syntax check"
    fi
}

check_workflow_structure() {
    echo ""
    echo "✓ Checking workflow structure..."
    
    local required_keys=("name" "on" "jobs")
    for key in "${required_keys[@]}"; do
        if grep -q "^${key}:" "$WORKFLOW_FILE"; then
            echo "  Found: $key"
        else
            echo "  Missing required key: $key"
            exit 1
        fi
    done
}

check_jobs() {
    echo ""
    echo "✓ Checking jobs..."
    
    local jobs=("detect_changes" "setup" "build_x86_64" "build_rpi5" "build_image")
    for job in "${jobs[@]}"; do
        if grep -q "^  ${job}:" "$WORKFLOW_FILE"; then
            echo "  Found job: $job"
        else
            echo "  Missing job: $job"
            exit 1
        fi
    done
}

check_workflow_dispatch() {
    echo ""
    echo "✓ Checking workflow_dispatch inputs..."
    
    local inputs=("skip_cache" "force_toolchain" "force_system" "force_packages" "architectures" "build_target")
    for input in "${inputs[@]}"; do
        if grep -q "input.*${input}" "$WORKFLOW_FILE" || grep -q "${input}:" "$WORKFLOW_FILE"; then
            echo "  Found input: $input"
        else
            echo "  Missing input: $input"
            exit 1
        fi
    done
}

check_cache_logic() {
    echo ""
    echo "✓ Checking cache logic..."
    
    if grep -q "restore-keys:" "$WORKFLOW_FILE"; then
        echo "  Found restore-keys configuration"
    else
        echo "  Warning: No restore-keys found"
    fi
    
    if grep -q "cache_status" "$WORKFLOW_FILE"; then
        echo "  Found cache_status output"
    else
        echo "  Warning: No cache_status output found"
    fi
}

check_detect_changes() {
    echo ""
    echo "✓ Checking detect_changes job..."
    
    local outputs=("needs_toolchain" "needs_system" "needs_packages" "needs_image" "diff_hash" "cache_status")
    for output in "${outputs[@]}"; do
        if grep -q "$output.*>>.*GITHUB_OUTPUT" "$WORKFLOW_FILE"; then
            echo "  Found output: $output"
        else
            echo "  Warning: Missing output $output in detect_changes"
        fi
    done
}

run_act_test() {
    echo ""
    echo "=========================================="
    echo "  Testing with act (optional)"
    echo "=========================================="
    
    if ! command -v act &> /dev/null; then
        echo "  act not installed. To install:"
        echo "  brew install act          # macOS"
        echo "  curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash  # Linux"
        echo ""
        echo "  Or run: act -W .github/workflows/build-recalbox.yml --dryrun"
        return 0
    fi
    
    echo "  Running act dry-run..."
    if act -W "$WORKFLOW_FILE" --dryrun 2>&1; then
        echo "  act dry-run: SUCCESS"
    else
        echo "  act dry-run: FAILED (this may be expected)"
    fi
}

show_usage() {
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --validate-only    Only validate YAML syntax"
    echo "  --full             Run full validation including act"
    echo "  --help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                 Run default validation"
    echo "  $0 --validate-only Only check YAML syntax"
    echo "  $0 --full          Run all tests including act"
}

main() {
    local mode="default"
    
    for arg in "$@"; do
        case $arg in
            --validate-only)
                mode="validate-only"
                ;;
            --full)
                mode="full"
                ;;
            --help|-h)
                show_usage
                exit 0
                ;;
        esac
    done
    
    check_yaml_syntax
    check_workflow_structure
    check_jobs
    check_workflow_dispatch
    check_cache_logic
    check_detect_changes
    
    if [ "$mode" = "full" ]; then
        run_act_test
    fi
    
    echo ""
    echo "=========================================="
    echo "  Validation Complete!"
    echo "=========================================="
    echo ""
    echo "To trigger workflow manually:"
    echo "  gh workflow run build-recalbox.yml"
    echo ""
    echo "Or use the GitHub UI:"
    echo "  https://github.com/$(gh repo view --json owner --jq '.owner.login')/$(gh repo view --json name --jq '.name')/actions/workflows/build-recalbox.yml"
}

main "$@"
