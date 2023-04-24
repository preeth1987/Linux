#linux aliases
#First source official aliases if present
if [ -f $HOME/.official_bash_aliases ]; then
	echo "sourcing official aliases"
	. $HOME/.official_bash_aliases
fi
alias cdb='. ~/bin/cdb'
export SCRIPT_DIR=$HOME/git/Linux/scripts
export PATH=$PATH:$SCRIPT_DIR
#alias findsize="tot=0;find . -type f | xargs ls -s | sort -rn | awk '{size=\$1/1024;tot=tot+\$1;printf \"%dMB -> %s\n\",size,\$2 } END { printf \"Total: %dKB\n\", tot }'"
alias findsize="du -h --max-depth=1"
alias latest="find . -maxdepth 1 -printf '%TY-%Tm-%Td %TT ' -exec du -sh {} \; | sort -r" 
alias clean_30="find . -type f -mtime +30 -exec rm -f {} \; && find . -type d -mtime +30 -exec rm -Rf {} \;"
alias tip="cat ~/tip | grep -i $1"
alias lm="find -printf \"%TY-%Tm-%Td %TT %p\n\" | sort -n"
#alias ll="ls -al | pg"
alias ls='ls --color=auto'
alias ls-tree='tree -L 2'
alias la='ls -A'
alias l='ls -CF'
alias t='telnet'
alias s='ssh'
alias search_replace="sed -i -- 's/searchPattern/replacePattern/g' *"
alias agrep="find . -name '*.[ch]' | xargs grep -i $1"
alias hgrep="find . -name '*.[h]' | xargs grep -i $1"
alias dumpIp="cut -d',' -f3 "
#My TTY, change according to OS
export MYTTY=xterm

#SERVER/VM configuration
alias svm='ssh $USER1@$VM1'
alias svm1='ssh $USER1@$VM1'

#RPI
alias rpi="ssh osmc@osmc"
alias oxu4="ssh root@oxu4"

#vim aliases
set -o vi
#alias vi='vim'
export EDITOR=vim

#Keybindings
alias ="clear"

#For vim backspace to work
#stty erase '^?'

del_ssh_key() {
	sed -i '/'$1'/d' ~/.ssh/known_hosts
}
alias dkey='del_ssh_key'

add_auto_login() {
    if [[ ${#} -lt 2 ]]
    then
        echo "Usage: add_auto_login <uname> <ip>"
        return
    fi
    del_ssh_key $2
    cat ~/.ssh/id_rsa.pub | ssh $1@$2 "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys2"
}

sendToAll() {
	for i in `dumpIp $1`; do echo "IP: $i CMD: $2"; $SCRIPT_DIR/sendCmd $i "$2"; done
}

#Mail alias
mailme() {
    if [[ ${#2} -eq 0 ]]
    then
        mail -s "SELF:$1" -c "$EMAIL; " $EMAIL < /dev/null
        return
    fi
    tail -50 $2 | tr -cd "[:print:]\n" | mailx -s "SELF:$1" -c "$EMAIL; " $EMAIL
}
export -f mailme
cpr() {
    if [[ ${#} -lt 2 ]]
    then
        echo "Usage: cpr source destination"
        echo "Copy recursively from source to destination"
        return
    fi
    #rsync --info=progress2 $1 $2
    rsync -avzP $1 $2
}

hint() {
    if [[ ${#} -lt 1 ]]
    then
        echo "Usage: hint <key>"
        return
    fi
    echo "Hint for $1"
    pat=$1
    awk '/<START>/ {p=1}; \
     {if (p==1) {a[NR]=$0}}; \
     /'"$pat"'/ {f=1}; \
     /<END>/ {p=0; if (f==1) {for (i in a) print a[i]};f=0; delete a}' $HOME/hints
}

#git shortcuts
alias cdgit='cd $HOME/git'
alias gitdir="$HOME/git"
if [ -f $HOME/git/Linux/bash_aliases/.git_aliases ]; then
	echo "Sourcing git aliases"
	source $HOME/git/Linux/bash_aliases/.git_aliases
fi

alias cscope='cscope -R'
expense() {
    if [[ ${#} -lt 2 ]]
    then
        echo "Usage: expense <file> <Type>"
        return
    fi
    grep -i $2 $1 | awk '{tot+=$NF} END {print tot}'
}

sort_bill() {
    if [[ ${#} -lt 1 ]]
    then
        echo "Usage: sort_bill <file>"
        return
    fi
    grep -iE "[a-z]+" $1 | awk '{print $NF,$0}' | sort -n | cut -f2- -d' ' | grep -v RECD
}

recursive_diff() {
    export pfx="/cygdrive/g/My Drive/"
    find "." -type f -print -exec diff "{}" "$pfx/{}" \;
}
