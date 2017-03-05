#linux aliases
alias cdb='. ~/bin/cdb'
SCRIPT_DIR=$HOME/scripts
export PATH=$PATH:$SCRIPT_DIR
alias findsize="tot=0;find . -type f | xargs ls -s | sort -rn | awk '{size=\$1/1024;tot=tot+\$1;printf \"%dMB -> %s\n\",size,\$2 } END { printf \"Total: %dKB\n\", tot }'"
alias tip="cat ~/tip | grep -i $1"
#alias ll="ls -al | pg"
alias la='ls -A'
alias l='ls -CF'
alias t='telnet'
alias s='ssh'
alias search_replace="sed -i -- 's/searchPattern/replacePattern/g' *"
alias agrep="find . -name '*.[ch]' | xargs grep -i $1"
alias hgrep="find . -name '*.[h]' | xargs grep -i $1"

alias svm="ssh_script"

#vim aliases
set -o vi
alias vi='vim'
export EDITOR=vim

#Keybindings
alias ="clear"

del_ssh_key() {
	sed -i '/'$1'/d' ~/.ssh/known_hosts
}
