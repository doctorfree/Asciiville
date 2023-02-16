## NeoMutt

### Table of Contents

1. [NeoMutt encrypted passwords](#neomutt-encrypted-passwords)
1. [NeoMutt Key Bindings](#neomutt-key-bindings)
1. [Replacing an existing NeoMutt configuration](#replacing-an-existing-neomutt-configuration)

### NeoMutt encrypted passwords

To use an encrypted password with NeoMutt, follow the guide at:
[https://www.xmodulo.com/mutt-email-client-encrypted-passwords.html](https://www.xmodulo.com/mutt-email-client-encrypted-passwords.html)
The ArchLinux wiki has a good article on configuring GPG to encrypt/decrypt
passwords and email. See
[https://wiki.archlinux.org/title/GnuPG](https://wiki.archlinux.org/title/GnuPG)

For example, if you have encrypted your Google NeoMutt App password in the
file `~/.neomutt/gmail.gpg` then the following can be used for GMail
authentication in `$HOME/.config/neomutt/accounts/gmail`

Create a GPG key pair
```console
gpg --gen-key
```

Create a file containing the encrypted password:
```console
echo "your_gmail_app_password" > ~/.config/neomutt/gmail
gpg -r youremail@gmail.com -e ~/.config/neomutt/gmail
rm -f ~/.config/neomutt/gmail
chmod 600 ~/.config/neomutt/gmail.gpg
```

Configure NeoMutt authentication in `$HOME/.config/neomutt/accounts/gmail`
```console
set imap_pass="`gpg --batch -q --decrypt ~/.neomutt/gmail.gpg`"
```

The first time `neomutt` is executed it will prompt you for the passphrase
used to create the GPG key pair. If you check the box to use the GPG
passphrase manager then subsequent invocations of NeoMutt will not prompt
for the passphrase. This assumes the `gpg-agent` has been installed and
configured. Many modern Linux distributions do this by default.

Creating and using encrypted passwords is strongly recommended. That's why
I spent the time to write this section of the README. A similar procedure
can be used with Mutt.

To transfer a previously generated GnuPG key pair from another system, see
[https://gist.github.com/angela-d/8b27670bac26e4bf7c431715fef5cc51](https://gist.github.com/angela-d/8b27670bac26e4bf7c431715fef5cc51)

### NeoMutt Key Bindings

The Asciiville NeoMutt configuration includes some custom key bindings
to ease NeoMutt navigation. These are documented in
`/usr/share/asciiville/neomutt/cheatsheet.md`.

The primary differences between the Asciiville NeoMutt key bindings
and the default are as follows:

- Vim keybinds. 'k' and 'j' to go up and down, and 'l' and 'h' to move forwards and back pages
- Open the sidebar with 'B'. 'Ctrl+j' and 'Ctrl+k' to navigate, 'Ctrl+o' to open selection
- 'A' will attempt to create a new contact with khard
- 'F2', 'F3', and 'F4' are used to switch between three mailboxes

The customized Asciiville NeoMutt key bindings are as follows:

#### Index

<table>
<thead>
<tr class="header">
<th>Function</th>
<th>Keybind</th>
</tr>
</thead>
<tbody>

<tr class="odd">
<td>Go to last entry</td>
<td><code>G</code></td>
</tr>
<tr class="even">
<td>Go to first entry</td>
<td><code>gg</code></td>
</tr>
<tr class="odd">
<td>Page down</td>
<td><code>d</code></td>
</tr>
<tr class="even">
<td>Page up</td>
<td><code>u</code></td>
</tr>
<tr class="odd">
<td>Delete message</td>
<td><code>D</code></td>
</tr>
<tr class="even">
<td>Delete thread</td>
<td><code>Ctrl+d</code></td>
</tr>
<tr class="odd">
<td>Mark thread read</td>
<td><code>Ctrl+r</code></td>
</tr>
<tr class="even">
<td>Undelete message</td>
<td><code>U</code></td>
</tr>
<tr class="odd">
<td>Limit/Filter</td>
<td><code>L</code></td>
</tr>
<tr class="even">
<td>Open message</td>
<td><code>l, Enter</code></td>
</tr>
<tr class="odd">
<td>Group reply</td>
<td><code>R</code></td>
</tr>
<tr class="even">
<td>Reply</td>
<td><code>r</code></td>
</tr>
<tr class="odd">
<td>Forward message</td>
<td><code>f</code></td>
</tr>
<tr class="even">
<td>New message</td>
<td><code>m</code></td>
</tr>
<tr class="odd">
<td>View attachments</td>
<td><code>v</code></td>
</tr>
<tr class="even">
<td>Compose to Sender</td>
<td><code>@</code></td>
</tr>
<tr class="odd">
<td>Resend message</td>
<td><code>Esc+e</code></td>
</tr>
<tr class="even">
<td>Save message (to file)</td>
<td><code>s</code></td>
</tr>
<tr class="odd">
<td>Toggle collapse thread</td>
<td><code>Space, Esc+v</code></td>
</tr>
<tr class="even">
<td>Toggle collapse all threads</td>
<td><code>Esc+V</code></td>
</tr>
<tr class="odd">
<td>Add contact to Khard</td>
<td><code>A</code></td>
</tr>
<tr class="even">
<td>Jump to parent message</td>
<td><code>P</code></td>
</tr>
<tr class="odd">
<td>Email PGP public key</td>
<td><code>Esc+k</code></td>
</tr>
</tbody>
</table>

#### Pager

<table>
<thead>
<tr class="header">
<th>Function</th>
<th>Keybind</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>Bottom of page</td>
<td><code>G</code></td>
</tr>
<tr class="even">
<td>Top of page</td>
<td><code>gg</code></td>
</tr>
<tr class="odd">
<td>Next line</td>
<td><code>j</code></td>
</tr>
<tr class="even">
<td>Previous line</td>
<td><code>k</code></td>
</tr>
<tr class="odd">
<td>View attachments</td>
<td><code>l</code></td>
</tr>
<tr class="even">
<td>Page down</td>
<td><code>d</code></td>
</tr>
<tr class="odd">
<td>Page up</td>
<td><code>u</code></td>
</tr>
<tr class="even">
<td>Group reply</td>
<td><code>R</code>,<code>g</code></td>
</tr>
<tr class="odd">
<td>Reply</td>
<td><code>r</code></td>
</tr>
<tr class="even">
<td>Compose to Sender</td>
<td><code>@</code></td>
</tr>
<tr class="odd">
<td>Delete thread</td>
<td><code>Ctrl+d</code></td>
</tr>
<tr class="even">
<td>Mark thread read</td>
<td><code>Ctrl+r</code></td>
</tr>
<tr class="odd">
<td>Mark thread new</td>
<td><code>N</code></td>
</tr>
</tbody>
</table>

#### Sidebar

<table>
<thead>
<tr class="header">
<th>Function</th>
<th>Keybind</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>Show sidebar</td>
<td><code>B</code></td>
</tr>
<tr class="even">
<td>Up</td>
<td><code>Ctrl+k</code></td>
</tr>
<tr class="odd">
<td>Down</td>
<td><code>Ctrl+j</code></td>
</tr>
<tr class="even">
<td>Open</td>
<td><code>Ctrl+o</code></td>
</tr>
<tr class="odd">
<td>Open next new</td>
<td><code>Ctrl+n</code></td>
</tr>
<tr class="even">
<td>Open previous new</td>
<td><code>Ctrl+p</code></td>
</tr>
</tbody>
</table>


### Replacing an existing NeoMutt configuration

Asciiville `ascinit` skips NeoMutt initialization and configuration if it
detects an existing `$HOME/.config/neomutt/` folder. If you have already
configured NeoMutt then `ascinit` does not touch the existing configuration.
However, you may want to examine the NeoMutt configuration provided in
Asciiville by viewing the files in `/usr/share/asciiville/neomutt/`. If you
want to use the Asciiville NeoMutt setup files rather than your previously
configured setup, move the existing `$HOME/.config/neomutt/` folder aside
and rerun `ascinit`.

