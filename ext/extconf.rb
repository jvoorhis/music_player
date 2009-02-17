require 'mkmf'

$CFLAGS << ' -Wall '
$LDFLAGS = '-framework AudioToolbox -framework CoreMIDI'

extname = 'music_player'

dir_config(extname)
create_makefile(extname)
