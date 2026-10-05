_my_cpp_complete_safe() {
    local cur="${COMP_WORDS[COMP_CWORD]}"
    
    # Safely populates COMPREPLY line-by-line, preserving spaces
    mapfile -t COMPREPLY < <($(pwd)/build/debug/bin/hypercube "$cur")
}
complete -F _my_cpp_complete_safe hypercube