#linux aliases
alias cdb='. ~/bin/cdb'
export SCRIPT_DIR=$HOME/git/Linux/scripts
export PATH=$PATH:$SCRIPT_DIR
alias findsize="tot=0;find . -type f | xargs ls -s | sort -rn | awk '{size=\$1/1024;tot=tot+\$1;printf \"%dMB -> %s\n\",size,\$2 } END { printf \"Total: %dKB\n\", tot }'"
alias tip="cat ~/tip | grep -i $1"
alias lm="find -printf \"%TY-%Tm-%Td %TT %p\n\" | sort -n"
#alias ll="ls -al | pg"
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
alias svm='ssh $USER@$VM'
alias svm1='ssh $USER1@$VM1'

#RPI
alias rpi="ssh osmc@osmc"

#vim aliases
set -o vi
alias vi='vim'
export EDITOR=vim

#Keybindings
alias ="clear"

#git shortcuts
alias cdgit='cd $HOME/git'
alias gitdir="$HOME/git"
alias gits="git status"
gitco ()
{
    git checkout $*
}
gitci ()
{
    git commit $* -m "commit";
    git push
}
gitr ()
{
    git pull origin master
}
gitlt ()
{
	git tag -l | less
}
gitst ()
{
	git checkout -b stable $*
}

del_ssh_key() {
	sed -i '/'$1'/d' ~/.ssh/known_hosts
}
sendToAll() {
	for i in `dumpIp $1`; do echo "IP: $i CMD: $2"; $SCRIPT_DIR/sendCmd $i "$2"; done
}
