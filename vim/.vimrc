"" Set terminal as xterm so that below mappings work
set term=xterm
"""" START OF PLUGINS

"execute pathogen#infect()
syntax on
filetype plugin indent on

"Vim git plugin
"Plug 'airblade/vim-gitgutter'

"""" END OF PLUGINS

set showmode
set sw=4
set notitle

set smartindent
set ai
set hls
set ic
set is
set ru
set sm
set vb
set smd
set showcmd
set showmatch
set nofoldenable
set backspace=indent,eol,start
set foldmethod=indent
set diffopt+=iwhite

highlight Search ctermfg=Black ctermbg=Yellow cterm=NONE
"highlight Statement ctermfg=DarkBlue
highlight Statement ctermfg=Green
highlight PreProc ctermfg=DarkMagenta
highlight Type ctermfg=Green
highlight Comment ctermfg=Grey

"function GetCscopeFileName()
"  let curdir = fnamemodify(getcwd(), ':p')
"  while 1
"    let f = curdir . (curdir =~ '[\\/]$' ? '' : '/') . 'cscope.out'
"    if filereadable(f) " found
"      return f
"    endif
"    " try one level up
"    let d = fnamemodify(curdir, ':h')
"    if d == curdir
"      " trying to go past top level: not found
"      return ''
"    endif
"    let curdir = d
"  endwhile
"endfunction 


" This tests to see if vim was configured with the '--enable-cscope' option
" when it was compiled.  If it wasn't, time to recompile vim... 
if has("cscope")

    """"""""""""" Standard cscope/vim boilerplate

    " use both cscope and ctag for 'ctrl-]', ':ta', and 'vim -t'
    set cscopetag

    " check cscope for definition of a symbol before checking ctags: set to 1
    " if you want the reverse search order.
    set csto=0

"    let csfn = GetCscopeFileName()
"    let dname = fnamemodify(csfn, ":s?/cscope.out??")

    " add any cscope database in current directory
	"if filereadable("~/cscope/cscope.out")
	"echo "add cscope"
        "cs add ~/cscope
    " else add the database pointed to by environment variable 
	" expecting CSCOPE_DB in format "<cscope db path>;<prepend prefix path>"
    if $CSCOPE_DB != ""
        for cscope_db in split($CSCOPE_DB, "&&")
            if cscope_db != ""
                "echo "cscope_db is " . cscope_db
                let db = strpart(cscope_db, 0, match(cscope_db, ";"))
                "echo "db is " . db
		        let prepend = strpart(matchstr(cscope_db, ";.*"), 1)
                "echo "prepend is " . prepend
                if (!empty(prepend) )
                    set nocscopeverbose
                    "echo "db: " . db . " path: " . prepend
                    exe "cs add " . db . " " . prepend
                else
                    set nocscopeverbose
                    exe "cs add " . db
                endif
            endif
        endfor
    endif

    " show msg when any other cscope db added
    "set cscopeverbose

    """"""""""""" My cscope/vim key mappings
    "
    " The following maps all invoke one of the following cscope search types:
    "
    "   's'   symbol: find all references to the token under cursor
    "   'g'   global: find global definition(s) of the token under cursor
    "   'c'   calls:  find all calls to the function name under cursor
    "   't'   text:   find all instances of the text under cursor
    "   'e'   egrep:  egrep search for the word under cursor
    "   'f'   file:   open the filename under cursor
    "   'i'   includes: find files that include the filename under cursor
    "   'd'   called: find functions that function under cursor calls
    "
    " Below are three sets of the maps: one set that just jumps to your
    " search result, one that splits the existing vim window horizontally and
    " diplays your search result in the new window, and one that does the same
    " thing, but does a vertical split instead (vim 6 only).
    "
    " I've used CTRL-\ and CTRL-@ as the starting keys for these maps, as it's
    " unlikely that you need their default mappings (CTRL-\'s default use is
    " as part of CTRL-\ CTRL-N typemap, which basically just does the same
    " thing as hitting 'escape': CTRL-@ doesn't seem to have any default use).
    " If you don't like using 'CTRL-@' or CTRL-\, , you can change some or all
    " of these maps to use other keys.  One likely candidate is 'CTRL-_'
    " (which also maps to CTRL-/, which is easier to type).  By default it is
    " used to switch between Hebrew and English keyboard mode.
    "
    " All of the maps involving the <cfile> macro use '^<cfile>$': this is so
    " that searches over '#include <time.h>" return only references to
    " 'time.h', and not 'sys/time.h', etc. (by default cscope will return all
    " files that contain 'time.h' as part of their name).


    " To do the first type of search, hit 'CTRL-\', followed by one of the
    " cscope search types above (s,g,c,t,e,f,i,d).  The result of your cscope
    " search will be displayed in the current window.  You can use CTRL-T to
    " go back to where you were before the search.  
    "

    nmap <C-\>s :cs find s <C-R>=expand("<cword>")<CR><CR>
    nmap <C-\>g :cs find g <C-R>=expand("<cword>")<CR><CR>
    nmap <C-\>c :cs find c <C-R>=expand("<cword>")<CR><CR>
    nmap <C-\>t :cs find t <C-R>=expand("<cword>")<CR><CR>
    nmap <C-\>e :cs find e <C-R>=expand("<cword>")<CR><CR> 
    nmap <C-\>f :cs find f <C-R>=expand("<cfile>")<CR><CR>   
    nmap <C-\>i :cs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
    nmap <C-\>d :cs find d <C-R>=expand("<cword>")<CR><CR>


    " Using 'CTRL-spacebar' (intepreted as CTRL-@ by vim) then a search type
    " makes the vim window split horizontally, with search result displayed in
    " the new window.
    "
    " (Note: earlier versions of vim may not have the :scs command, but it
    " can be simulated roughly via:
    "    nmap <C-@>s <C-W><C-S> :cs find s <C-R>=expand("<cword>")<CR><CR>      

    nmap <C-@>s :scs find s <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@>g :scs find g <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@>c :scs find c <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@>t :scs find t <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@>e :scs find e <C-R>=expand("<cword>")<CR><CR> 
    nmap <C-@>f :scs find f <C-R>=expand("<cfile>")<CR><CR>   
    nmap <C-@>i :scs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
    nmap <C-@>d :scs find d <C-R>=expand("<cword>")<CR><CR>

    " Hitting CTRL-space *twice* before the search type does a vertical 
    " split instead of a horizontal one (vim 6 and up only)
    "
    " (Note: you may wish to put a 'set splitright' in your .vimrc
    " if you prefer the new window on the right instead of the left

    nmap <C-@><C-@>s :vert scs find s <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@><C-@>g :vert scs find g <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@><C-@>c :vert scs find c <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@><C-@>t :vert scs find t <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@><C-@>e :vert scs find e <C-R>=expand("<cword>")<CR><CR>
    nmap <C-@><C-@>f :vert scs find f <C-R>=expand("<cfile>")<CR><CR>
    nmap <C-@><C-@>i :vert scs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
    nmap <C-@><C-@>d :vert scs find d <C-R>=expand("<cword>")<CR><CR>


    """"""""""""" key map timeouts
    "
    " By default Vim will only wait 1 second for each keystroke in a mapping.
    " You may find that too short with the above typemaps.  If so, you should
    " either turn off mapping timeouts via 'notimeout'.
    "
    "set notimeout 
    "
    " Or, you can keep timeouts, by uncommenting the timeoutlen line below,
    " with your own personal favorite value (in milliseconds):
    "
    "set timeoutlen=4000
    "
    " Either way, since mapping timeout settings by default also set the
    " timeouts for multicharacter 'keys codes' (like <F1>), you should also
    " set ttimeout and ttimeoutlen: otherwise, you will experience strange
    " delays as vim waits for a keystroke after you hit ESC (it will be
    " waiting to see if the ESC is actually part of a key code like <F1>).
    "
    "set ttimeout 
    "
    " personally, I find a tenth of a second to work well for key code
    " timeouts. If you experience problems and have a slow terminal or network
    " connection, set it higher.  If you don't set ttimeoutlen, the value for
    " timeoutlent (default: 1000 = 1 second, which is sluggish) is used.
    "
    "set ttimeoutlen=100

endif

highlight DiffAdd term=reverse cterm=bold ctermbg=green ctermfg=white 
highlight DiffChange term=reverse cterm=bold ctermbg=cyan ctermfg=black 
highlight DiffText term=reverse cterm=bold ctermbg=gray ctermfg=black 
highlight DiffDelete term=reverse cterm=bold ctermbg=red ctermfg=black
set term=xterm

" function F2 key will display the tab insertions
nnoremap    <F2> :<C-U>setlocal lcs=tab:>-,trail:-,eol:$ list! list? <CR>

"call plug#begin('~/.vim/plugged')

" Declare the list of plugins.
" Plug 'whiteinge/diffconflicts'
" Plug 'tpope/vim-fireplace'
" Git vim fugitive plugin
"Plug 'git://github.com/tpope/vim-fugitive.git'
" vim diff conflict tool Use cmd :DiffConflictsShowHistory
"Plug 'git://github.com/whiteinge/diffconflicts.git'
" Python autocomplte 
" Plug 'git://github.com/davidhalter/jedi-vim.git'
" List ends here. Plugins become visible to Vim after this call.
"call plug#end()
"func Backspace()
"  if col('.') == 1
"    if line('.')  != 1
"      return  "\<ESC>kA\<Del>"
"    else
"      return ""
"    endif
"  else
"    return "\<Left>\<Del>"
"  endif
"endfunc

"inoremap <BS> <c-r>=Backspace()<CR>

syntax on
set ruler
"filetype indent plugin on
" configure expanding of tabs for various file types
" au BufRead,BufNewFile *.py set noexpandtab
" au BufRead,BufNewFile *.c set noexpandtab
" au BufRead,BufNewFile *.h set noexpandtab
" au BufRead,BufNewFile Makefile* set noexpandtab

" --------------------------------------------------------------------------------
" configure editor with tabs and nice stuff...
" --------------------------------------------------------------------------------
"set expandtab           " enter spaces when tab is pressed
"set textwidth=120       " break lines when line length increases
"set tabstop=4           " use 4 spaces to represent tab
"set softtabstop=4
"set shiftwidth=4        " number of spaces to use for auto indent
set autoindent          " copy indent from current line when starting a new line

" :retab changes *everything*, not just start of lines
fun! Retab(expandtab)
    let l:spaces = repeat(' ', &tabstop)

    " Replace tabs with spaces
    if a:expandtab
        silent! execute '%substitute#^\%(' . l:spaces . '\)\+#\=repeat("\t", len(submatch(0)) / &tabstop)#e'
    " Replace spaces with tabs
    else
        silent! execute '%substitute#^\%(\t\)\+#\=repeat("' . l:spaces . '", len(submatch(0)))#e'
    endif
endfun
fun! LoadPlugin()
    execute pathogen#infect()
    autocmd VimEnter * GitGutterEnable
endfun
filetype plugin indent on
if has('terminal')
  tnoremap <S-Space> <Space>
endif
map <C-J> '<,'>s/,/,\r/g
map <ESC>[8~    <End>

map <ESC>[7~    <Home>
:command Autosave autocmd TextChanged,TextChangedI * silent update 
:command LoadPlugin call LoadPlugin()
:command Gitg GitGutterToggle
"autocmd VimEnter * GitGutterDisable
