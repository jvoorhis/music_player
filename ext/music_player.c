#include <ruby.h>
#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/MIDIServices.h>

/* Ruby type decls */

static VALUE rb_mCoreMIDI = Qnil;
static VALUE rb_mAudioToolbox = Qnil;
static VALUE rb_cMusicPlayer = Qnil;
static VALUE rb_cMusicSequence = Qnil;
static VALUE rb_cMusicTrack = Qnil;
static VALUE rb_cMIDINoteMessage = Qnil;
static VALUE rb_cMIDIChannelMessage = Qnil;

/* CoreMIDI defns */

static VALUE
core_midi_get_number_of_destinations (VALUE mod)
{
    return UINT2NUM(MIDIGetNumberOfDestinations());
}

static VALUE
core_midi_get_destination (VALUE mod, VALUE idx)
{
    ItemCount ic = NUM2UINT(idx);
    MIDIEndpointRef ref = MIDIGetDestination(ic);
    if (NULL == ref) { return Qnil; }
    return ULONG2NUM((UInt32) ref);
}

/* MusicPlayer defns */

static void
player_free (MusicPlayer *player)
{
    OSStatus err;
    require_noerr( err = DisposeMusicPlayer(*player), fail );
    
    fail:
    rb_warning("DisposeMusicPlayer() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_new (VALUE class)
{
    OSStatus err;
    MusicPlayer *player = ALLOC(MusicPlayer);
    VALUE rb_player;
    
    require_noerr( err = NewMusicPlayer(player), fail );
    rb_player = Data_Wrap_Struct(class, 0, player_free, player);
    rb_obj_call_init(rb_player, 0, 0);
    return rb_player;
    
    fail:
    rb_raise(rb_eRuntimeError, "NewMusicPlayer() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_is_playing (VALUE self)
{
    MusicPlayer *player;
    Boolean playing;
    OSStatus err;
    
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerIsPlaying(*player, &playing), fail );
    return playing ? Qtrue : Qfalse;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerIsPlaying() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_get_sequence (VALUE self)
{
    return rb_iv_get(self, "@sequence");
}

static VALUE
player_set_sequence (VALUE self, VALUE rb_seq)
{
    MusicPlayer *player;
    MusicSequence *seq;
    OSStatus err;
    
    Data_Get_Struct(self, MusicPlayer, player);
    Data_Get_Struct(rb_seq, MusicSequence, seq);
    rb_iv_set(self, "@sequence", rb_seq);
    
    require_noerr( err = MusicPlayerSetSequence(*player, *seq), fail );
    return rb_seq;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerSetSequence() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_start (VALUE self)
{
    MusicPlayer *player;
    OSStatus err;
    
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerStart(*player), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerStart() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_stop (VALUE self)
{
    MusicPlayer *player;
    OSStatus err;
    
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerStop(*player), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerStop() failed with OSStatus %i.", (SInt32) err);
}

/* Sequence defns */

static void
sequence_free (MusicSequence *seq)
{
    OSStatus err;
    require_noerr( err = DisposeMusicSequence(*seq), fail );
    return;
    
    fail:
    rb_warning("DisposeMusicSequence() failed with %i.", (SInt32) err);
}

static VALUE
sequence_new (VALUE class)
{
    MusicSequence *seq = ALLOC(MusicSequence);
    OSStatus err;
    VALUE rb_seq;
    
    require_noerr( err = NewMusicSequence(seq), fail );
    rb_seq = Data_Wrap_Struct(class, 0, sequence_free, seq);
    rb_obj_call_init(rb_seq, 0, 0);
    return rb_seq;
    
    fail:
    rb_raise(rb_eRuntimeError, "NewMusicSequence() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
sequence_set_midi_endpoint (VALUE self, VALUE endpoint_ref)
{
    if (NIL_P(endpoint_ref)) { return Qnil; }
    
    MusicSequence *seq;
    UInt32 ref = NUM2ULONG(endpoint_ref);
    OSStatus err;
    
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = MusicSequenceSetMIDIEndpoint(*seq, (MIDIEndpointRef) ref), fail);
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceSetMIDIEndpoint() failed with OSStatus %i.", (SInt32) err);
}

/* Track defns */

static VALUE
track_init (VALUE self, VALUE rb_seq)
{
    rb_iv_set(self, "@sequence", rb_seq);
    return self;
}

static VALUE
track_new (VALUE class, VALUE rb_seq)
{
    MusicSequence *seq;
    MusicTrack *track = ALLOC(MusicTrack);
    OSStatus err;
    VALUE rb_track;
    VALUE argv[1];
    
    Data_Get_Struct(rb_seq, MusicSequence, seq);
    require_noerr( err = MusicSequenceNewTrack(*seq, track), fail );
    rb_track = Data_Wrap_Struct(class, 0, 0, track);
    argv[0] = rb_seq;
    rb_obj_call_init(rb_track, 1, argv);
    return rb_track;

    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceNewTrack() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
track_add_midi_note_message (VALUE self, VALUE at, VALUE rb_msg)
{
    MusicTrack *track;
    MIDINoteMessage *msg;
    MusicTimeStamp ts = (MusicTimeStamp) NUM2DBL(at);
    OSStatus err;
    
    Data_Get_Struct(self, MusicTrack, track);
    Data_Get_Struct(rb_msg, MIDINoteMessage, msg);
    require_noerr( err = MusicTrackNewMIDINoteEvent(*track, ts, msg), fail );
    return Qnil;

    fail:
    rb_raise(rb_eRuntimeError, "MusicTrackNewMIDINoteEvent() failed with OSStatus %i.", (SInt32) err);
}

/* MIDINoteMessage */

static void
midi_note_message_free (MIDINoteMessage *msg)
{
    free(msg);
}

static VALUE
midi_note_message_new (VALUE class, VALUE opts)
{
    if (T_HASH != TYPE(opts)) {
        rb_raise(rb_eTypeError, "Expected opts to be a Hash.");
    }
    
    MIDINoteMessage *msg = ALLOC(MIDINoteMessage);
    VALUE optChn, optNote, optVel, optRelVel, optDur;
    
    optChn = rb_hash_aref(opts, ID2SYM(rb_intern("channel")));
    msg->channel = (UInt8) FIXNUM_P(optChn) ? FIX2UINT(optChn) : 1;
    
    optNote = rb_hash_aref(opts, ID2SYM(rb_intern("pitch")));
    if (FIXNUM_P(optNote)) {
        msg->note = (UInt8) FIX2UINT(optNote);
    } else {
        rb_raise(rb_eArgError, ":pitch is required.");
    }
    
    optVel = rb_hash_aref(opts, ID2SYM(rb_intern("velocity")));
    msg->velocity = (UInt8) FIXNUM_P(optVel) ? FIX2UINT(optVel) : 64;
    
    optRelVel = rb_hash_aref(opts, ID2SYM(rb_intern("release_velocity")));
    msg->releaseVelocity = (UInt8) FIXNUM_P(optRelVel) ? FIX2UINT(optRelVel) : 0;
    
    optDur = rb_hash_aref(opts, ID2SYM(rb_intern("duration")));
    msg->duration = (MusicTimeStamp) (T_FLOAT == TYPE(optDur) || T_FIXNUM == TYPE(optDur)) ? NUM2DBL(optDur) : 1.0;
    
    VALUE rb_msg = Data_Wrap_Struct(class, 0, midi_note_message_free, msg);
    rb_obj_call_init(rb_msg, 0, 0);
    return rb_msg;
}

/* MIDIChannelMessage */

static void
midi_channel_message_free (MIDIChannelMessage *msg)
{
    free(msg);
}

static VALUE
midi_channel_message_new (VALUE class, VALUE opts)
{
    if (T_HASH != TYPE(opts)) {
        rb_raise(rb_eTypeError, "Expected opts to be a Hash.");
    }
    
    MIDIChannelMessage *msg = ALLOC(MIDIChannelMessage);
    VALUE optStatus, optData1, optData2;
    
    optStatus = rb_hash_aref(opts, ID2SYM(rb_intern("status")));
    if (!FIXNUM_P(optStatus)) {
        rb_raise(rb_eArgError, "Missing :status argument.");
    } else {
        msg->status = NUM2DBL(optStatus);
    }
    
    optData1 = rb_hash_aref(opts, ID2SYM(rb_intern("data1")));
    if (!NIL_P(optData1)) { msg->data1 = (UInt8) FIX2INT(optData1); }
    
    optData2 = rb_hash_aref(opts, ID2SYM(rb_intern("data2")));
    if (!NIL_P(optData2)) { msg->data2 = (UInt8) FIX2INT(optData2); }
    
    return Data_Wrap_Struct(class, 0, midi_channel_message_free, msg);
}

/* Initialize extension */

void
Init_music_player ()
{
    /*
     * CoreMIDI
     */
    rb_mCoreMIDI = rb_define_module("CoreMIDI");
    rb_define_module_function(rb_mCoreMIDI, "get_number_of_destinations",
                              core_midi_get_number_of_destinations, 0);
    rb_define_module_function(rb_mCoreMIDI, "get_destination",
                              core_midi_get_destination, 1);
    
    /*
     * AudioToolbox
     */
    rb_mAudioToolbox = rb_define_module("AudioToolbox");
    
    /* AudioToolbox::MusicPlayer */
    rb_cMusicPlayer = rb_define_class_under(rb_mAudioToolbox, "MusicPlayer", rb_cObject);
    rb_define_singleton_method(rb_cMusicPlayer, "new", player_new, 0);
    rb_define_method(rb_cMusicPlayer, "playing?", player_is_playing, 0);
    rb_define_method(rb_cMusicPlayer, "sequence", player_get_sequence, 0);
    rb_define_method(rb_cMusicPlayer, "sequence=", player_set_sequence, 1);
    rb_define_method(rb_cMusicPlayer, "start", player_start, 0);
    rb_define_method(rb_cMusicPlayer, "stop", player_stop, 0);
    
    /* AudioToolbox::MusicSequence */
    rb_cMusicSequence = rb_define_class_under(rb_mAudioToolbox, "MusicSequence", rb_cObject);
    rb_define_singleton_method(rb_cMusicSequence, "new", sequence_new, 0);
    rb_define_method(rb_cMusicSequence, "midi_endpoint=", sequence_set_midi_endpoint, 1);
    
    /* AudioToolbox::MusicTrack */
    rb_cMusicTrack = rb_define_class_under(rb_mAudioToolbox, "MusicTrack", rb_cObject);
    rb_define_singleton_method(rb_cMusicTrack, "new", track_new, 1);
    rb_define_method(rb_cMusicTrack, "initialize", track_init, 1);
    rb_define_method(rb_cMusicTrack, "add_midi_note_message", track_add_midi_note_message, 2);
    
    /* AudioToolbox::MIDINoteMessage */
    rb_cMIDINoteMessage = rb_define_class_under(rb_mAudioToolbox, "MIDINoteMessage", rb_cObject);
    rb_define_singleton_method(rb_cMIDINoteMessage, "new", midi_note_message_new, 1);
    
    /* AudioToolbox::MIDIChannelMessage */
    rb_cMIDIChannelMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIChannelMessage", rb_cObject);
    rb_define_singleton_method(rb_cMIDIChannelMessage, "new", midi_channel_message_new, 1);
}
