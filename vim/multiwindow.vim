function! OpenSplitSsh(command)
    execute 'split | terminal ssh ' . a:command
endfunction

command! -nargs=1 Ssh :call OpenSplitSsh(<q-args>)


function! MultipleSsh(...)
  " Define the number of windows you want to open
  let num_windows = 4

  " Close all existing windows
  silent! bufdo bwipeout!

  " Create new terminal windows and execute the commands
  let i = 0
  while i < num_windows
    " Open a new window and execute the commands in terminal
    execute 'split | terminal'
    for command in a:000
      let escaped_command = substitute(a:command, '|', '\|', 'g')
      execute "normal! i" . escaped_command
      execute "normal! \<CR>"
    endfor

    " Increase the counter
    let i += 1
  endwhile
endfunction

" Define a command to accept a list of commands as arguments
command! -nargs=* MultipleSsh :call MultipleSsh(<f-args>)

"function! MultipleSsh(commands)
"endfunction

"command! -nargs=+ MultipleSsh :call MultipleSsh(split(<q-args>, '\|')) 

function! OpenMultipleWindowsWithCommand(command)
  " Define the number of windows you want to open
  let num_windows = 4

  " Close all existing windows
  silent! bufdo bwipeout!

  " Create new terminal windows and execute the command
  let i = 0
  while i < num_windows
    " Open a new window and execute the command in terminal
    execute 'split | terminal ' . a:command

    " Increase the counter
    let i += 1
  endwhile
endfunction

" Define a command to accept a command argument
command! -nargs=1 OpenMultipleWindowsWithCommand :call OpenMultipleWindowsWithCommand(<q-args>)



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

