function! GetWords(text)
    " Replace all whitespace characters (including tabs, spaces, etc.) with a single space
    let cleaned_text = substitute(a:text, '\s\+', ' ', 'g')

    " Split the text into words using a single space as the delimiter
    let words = split(cleaned_text, ' ')

    " Remove any empty strings from the list (resulting from consecutive spaces)
    let non_empty_words = filter(words, 'v:val != ""')

    " Get the count of non-empty words
    let word_count = len(non_empty_words)

    return non_empty_words 
endfunction

" Rotate a window horizontally to the left
function! RotateLeft()
    let l:curbuf = bufnr('%')
    hide
    wincmd h
    split
    exe 'buf' l:curbuf
endfunc
command! -nargs=* RotateLeft :call RotateLeft(<f-args>)

" Rotate a window horizontally to the right
function! RotateRight()
    let l:curbuf = bufnr('%')
    hide
    wincmd l
    split
    exe 'buf' l:curbuf
endfunc
command! -nargs=* RotateRight :call RotateRight(<f-args>)

" Rotate a window vertically down
function! RotateDown()
    let l:curbuf = bufnr('%')
    hide
    wincmd j
    split
    exe 'buf' l:curbuf
endfunc
command! -nargs=* RotateDown :call RotateDown(<f-args>)

" Rotate a window vertically up
function! RotateUp()
    let l:curbuf = bufnr('%')
    hide
    wincmd k
    split
    exe 'buf' l:curbuf
endfunc
command! -nargs=* RotateUp :call RotateUp(<f-args>)

function! SplitWindow(...)
  " Define the number of windows you want to open
  let num_windows = a:000[0]

  let cmd_list = GetWords(a:000[1])
  let cmd_len = (len(cmd_list))/num_windows
  "echom cmd_list
  "echom cmd_len

  let lsplit = num_windows 
  " Close all existing windows
  silent! bufdo bwipeout!

  " If more than 4 windows vertical split
  if num_windows >= 4
    execute 'vertical new'
    let lsplit = num_windows/2
  endif

  let i = 0
  let vsplit_count = 0
  let word_count = 0
  
  while i < num_windows
    "Get the command from the arguments
    let cmd_count = 0
    let command = ""
    while cmd_count < cmd_len
	let command = command . cmd_list[word_count]
	let command = command . " "
	let cmd_count += 1
	let word_count += 1
    endwhile

    " Open a new window and execute the cmd in terminal
    if i <= lsplit
       execute 'split | terminal ++curwin ' . command
       call RotateDown()
    else
       if vsplit_count == 0
         " Close old window in vertical split
         wincmd p
         close!
	 " Rotate to right old windows
         call RotateRight()
         let vsplit_count += 1
       endif
       execute 'split | terminal ++curwin ' . command
       call RotateDown()
    endif

    " Increase the counter
    let i += 1
  endwhile
  " Close old window in vertical split
  wincmd p
  close!
endfunction


function! SplitNameWindow(...)
  " Define the number of windows you want to open
  let num_windows = a:000[0]

  let cmd_list = GetWords(a:000[1])
  let cmd_len = (len(cmd_list))/num_windows
  "echom cmd_list
  "echom cmd_len

  let lsplit = num_windows 
  " Close all existing windows
  silent! bufdo bwipeout!

  " If more than 4 windows vertical split
  if num_windows >= 4
    execute 'vertical new'
    let lsplit = num_windows/2
  endif

  let i = 0
  let vsplit_count = 0
  let word_count = 0
  
  while i < num_windows
    "Get the command from the arguments
    let cmd_count = 0
    let command = ""
    let window_name = "NONAME"
    while cmd_count < cmd_len
	if cmd_list[word_count] == "NAME:"
	    let window_name = cmd_list[word_count+1]
	    let cmd_count += 2
	    let word_count += 2
	else
	    let command = command . cmd_list[word_count]
	    let command = command . " "
	    let cmd_count += 1
	    let word_count += 1
	endif
    endwhile

    " Open a new window and execute the cmd in terminal
    if i <= lsplit
       execute 'split | terminal ++curwin ' . command
       execute 'setl stl=' . window_name
       call RotateDown()
    else
       if vsplit_count == 0
         " Close old window in vertical split
         wincmd p
         close!
	 " Rotate to right old windows
         call RotateRight()
         let vsplit_count += 1
       endif
       execute 'split | terminal ++curwin ' . command
       execute 'setl stl=' . window_name
       call RotateDown()
    endif

    " Increase the counter
    let i += 1
  endwhile
  " Close old window in vertical split
  wincmd p
  close!
endfunction

function! MultipleWindows(command)
    let num_windows = a:command
    let cmd_list = ""
    for i in range(0, num_windows - 1)
	let cmd_list = cmd_list . 'bash'
    endfor
    call SplitWindow(num_windows, cmd_list)
endfunction

" Define a command to accept a command argument
command! -nargs=1 MultipleWindows :call MultipleWindows(<q-args>)


function! OpenMultipleWindowsSsh(...)
    let num_windows = len(a:000)
    let cmd_list = ""
    for i in range(0, num_windows - 1)
	let command = " ssh admin@" . a:000[i]
	let cmd_list = cmd_list . command
    endfor
    call SplitWindow(num_windows, cmd_list)
endfunction

command! -nargs=* OpenMultipleWindowsSsh :call OpenMultipleWindowsSsh(<f-args>)

function! OpenMultipleNameWindowsSsh(...)
    let num_windows = len(a:000)/2
    let cmd_list = ""
    let i = 0
    while i < len(a:000)
	let namecmd = " NAME: " . a:000[i]
	let command = " ssh admin@" . a:000[i+1]
	let cmd_list = cmd_list . namecmd 
	let cmd_list = cmd_list . command
	let i += 2
    endwhile
    echom cmd_list
    " call feedkeys("\<CR>")
    call SplitNameWindow(num_windows, cmd_list)
endfunction

command! -nargs=* OpenMultipleNameWindowsSsh :call OpenMultipleNameWindowsSsh(<f-args>)

function! OpenMultipleWindowsCon(...)
    let num_windows = len(a:000)/2
    let cmd_list = ""
    let i=0
    while i < len(a:000)
	let command = " telnet " . a:000[i] . " " . a:000[i+1]
	let cmd_list = cmd_list . command
	let i += 2
    endwhile
    echom cmd_list
    call feedkeys("\<CR>")
    call SplitWindow(num_windows, cmd_list)
endfunction

command! -nargs=* OpenMultipleWindowsCon :call OpenMultipleWindowsCon(<f-args>)

function! OpenMultipleNameWindowsCon(...)
    let num_windows = len(a:000)/3
    let cmd_list = ""
    let i=0
    while i < len(a:000)
	let namecmd = " NAME: " . a:000[i]
	let command = " telnet " . a:000[i+1] . " " . a:000[i+2]
	let cmd_list = cmd_list . namecmd 
	let cmd_list = cmd_list . command
	let i += 3
    endwhile
    echom cmd_list
    call feedkeys("\<CR>")
    call SplitNameWindow(num_windows, cmd_list)
endfunction

command! -nargs=* OpenMultipleNameWindowsCon :call OpenMultipleNameWindowsCon(<f-args>)

function! TermList() abort
  let term_bufs = []
  for i in range(1, tabpagewinnr(tabpagenr('$')))
    let bufnr = winbufnr(i)
    if getbufvar(bufnr, '&buftype') ==# 'terminal'
      call add(term_bufs, bufnr)
    endif
  endfor
  return term_bufs
endfunction

function! SendCommandToTerminals(command) abort
  let term_bufs = TermList()

  for bufnr in term_bufs
    call term_sendkeys(bufnr, a:command . "\<CR>")
  endfor
endfunction
command! -nargs=1 Cmdall :call SendCommandToTerminals(<q-args>)

