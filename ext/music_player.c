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
    return;
    
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
    VALUE rb_track, argv[1];
    
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
track_add_midi_note_message (VALUE self, VALUE rb_at, VALUE rb_msg)
{
    MusicTrack *track;
    MIDINoteMessage *msg;
    MusicTimeStamp ts = (MusicTimeStamp) NUM2DBL(rb_at);
    OSStatus err;
    
    Data_Get_Struct(self, MusicTrack, track);
    Data_Get_Struct(rb_msg, MIDINoteMessage, msg);
    require_noerr( err = MusicTrackNewMIDINoteEvent(*track, ts, msg), fail );
    return Qnil;

    fail:
    rb_raise(rb_eRuntimeError, "MusicTrackNewMIDINoteEvent() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
track_add_midi_channel_message (VALUE self, VALUE rb_at, VALUE rb_msg)
{
    MusicTrack *track;
    MIDIChannelMessage *msg;
    MusicTimeStamp ts = (MusicTimeStamp) NUM2DBL(rb_at);
    OSStatus err;
    
    Data_Get_Struct(self, MusicTrack, track);
    Data_Get_Struct(rb_msg, MIDIChannelMessage, msg);
    require_noerr( err = MusicTrackNewMIDIChannelEvent(*track, ts, msg), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicTrackNewMIDIChannelEvent() failed with OSStatus %i.", (SInt32) err);
}

/* MIDINoteMessage */

static VALUE
midi_note_message_channel (VALUE self)
{
    MIDINoteMessage *msg;
    Data_Get_Struct(self, MIDINoteMessage, msg);
    return UINT2NUM(msg->channel);
}

static VALUE
midi_note_message_note (VALUE self)
{
    MIDINoteMessage *msg;
    Data_Get_Struct(self, MIDINoteMessage, msg);
    return UINT2NUM(msg->note);
}

static VALUE
midi_note_message_velocity (VALUE self)
{
    MIDINoteMessage *msg;
    Data_Get_Struct(self, MIDINoteMessage, msg);
    return UINT2NUM(msg->velocity);
}

static VALUE
midi_note_message_release_velocity (VALUE self)
{
    MIDINoteMessage *msg;
    Data_Get_Struct(self, MIDINoteMessage, msg);
    return UINT2NUM(msg->releaseVelocity);
}

static VALUE
midi_note_message_duration (VALUE self)
{
    MIDINoteMessage *msg;
    Data_Get_Struct(self, MIDINoteMessage, msg);
    return UINT2NUM(msg->duration);
}

static void
midi_note_message_free (MIDINoteMessage *msg)
{
    free(msg);
}

static VALUE
midi_note_message_init (VALUE self, VALUE rb_opts)
{
    if (T_HASH != TYPE(rb_opts))
        rb_raise(rb_eArgError, "Expected argument to be a Hash.");

    MIDINoteMessage *msg;
    VALUE rb_chn, rb_note, rb_vel, rb_rel_vel, rb_dur;

    Data_Get_Struct(self, MIDINoteMessage, msg);

    rb_chn = rb_hash_aref(rb_opts, ID2SYM(rb_intern("channel")));
    msg->channel = FIXNUM_P(rb_chn) ? FIX2UINT(rb_chn) : 1;
    
    rb_note = rb_hash_aref(rb_opts, ID2SYM(rb_intern("note")));
    if (FIXNUM_P(rb_note))
        msg->note = FIX2UINT(rb_note);
    else
        rb_raise(rb_eArgError, ":note is required.");
    
    rb_vel = rb_hash_aref(rb_opts, ID2SYM(rb_intern("velocity")));
    msg->velocity = FIXNUM_P(rb_vel) ? FIX2UINT(rb_vel) : 64;
    
    rb_rel_vel = rb_hash_aref(rb_opts, ID2SYM(rb_intern("release_velocity")));
    msg->releaseVelocity = FIXNUM_P(rb_rel_vel) ? FIX2UINT(rb_rel_vel) : 0;
    
    rb_dur = rb_hash_aref(rb_opts, ID2SYM(rb_intern("duration")));
    msg->duration = (MusicTimeStamp) (T_FLOAT == TYPE(rb_dur) || T_FIXNUM == TYPE(rb_dur)) ? NUM2DBL(rb_dur) : 1.0;
    
    return self;
}

static VALUE
midi_note_message_new (VALUE class, VALUE rb_opts)
{
    MIDINoteMessage *msg = ALLOC(MIDINoteMessage);
    VALUE rb_msg, argv[1];
    
    rb_msg = Data_Wrap_Struct(class, 0, midi_note_message_free, msg);
    argv[0] = rb_opts;
    rb_obj_call_init(rb_msg, 1, argv);
    return rb_msg;
}

/* MIDIChannelMessage */

static VALUE
midi_channel_message_status (VALUE self)
{
    MIDIChannelMessage *msg;
    Data_Get_Struct(self, MIDIChannelMessage, msg);
    return UINT2NUM(msg->status);
}

static VALUE
midi_channel_message_data1 (VALUE self)
{
    MIDIChannelMessage *msg;
    Data_Get_Struct(self, MIDIChannelMessage, msg);
    return UINT2NUM(msg->data1);
}

static VALUE
midi_channel_message_data2 (VALUE self)
{
    MIDIChannelMessage *msg;
    Data_Get_Struct(self, MIDIChannelMessage, msg);
    return UINT2NUM(msg->data2);
}

static void
midi_channel_message_free (MIDIChannelMessage *msg)
{
    free(msg);
}

static VALUE
midi_channel_message_init (VALUE self, VALUE rb_opts)
{
    if (T_HASH != TYPE(rb_opts))
        rb_raise(rb_eArgError, "Expected argument to be a Hash.");
    
    MIDIChannelMessage *msg;
    VALUE rb_status, rb_data1, rb_data2;
    
    Data_Get_Struct(self, MIDIChannelMessage, msg);
    
    rb_status = rb_hash_aref(rb_opts, ID2SYM(rb_intern("status")));
    if (!FIXNUM_P(rb_status))
        rb_raise(rb_eArgError, ":status is required.");
    else
        msg->status = NUM2DBL(rb_status);
    
    rb_data1 = rb_hash_aref(rb_opts, ID2SYM(rb_intern("data1")));
    if (!NIL_P(rb_data1)) msg->data1 = (UInt8) FIX2INT(rb_data1);
    
    rb_data2 = rb_hash_aref(rb_opts, ID2SYM(rb_intern("data2")));
    if (!NIL_P(rb_data2)) msg->data2 = (UInt8) FIX2INT(rb_data2);
    
    return self;
}

static VALUE
midi_channel_message_new (VALUE class, VALUE rb_opts)
{
    MIDIChannelMessage *msg = ALLOC(MIDIChannelMessage);
    VALUE rb_msg, argv[1];
    
    rb_msg = Data_Wrap_Struct(class, 0, midi_channel_message_free, msg);
    argv[0] = rb_opts;
    rb_obj_call_init(rb_msg, 1, argv);
    return rb_msg;
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
    rb_define_method(rb_cMusicTrack, "add_midi_channel_message", track_add_midi_channel_message, 2);
    
    /* AudioToolbox::MIDINoteMessage */
    rb_cMIDINoteMessage = rb_define_class_under(rb_mAudioToolbox, "MIDINoteMessage", rb_cObject);
    rb_define_singleton_method(rb_cMIDINoteMessage, "new", midi_note_message_new, 1);
    rb_define_method(rb_cMIDINoteMessage, "initialize", midi_note_message_init, 1);
    rb_define_method(rb_cMIDINoteMessage, "channel", midi_note_message_channel, 0);
    rb_define_method(rb_cMIDINoteMessage, "note", midi_note_message_note, 0);
    rb_define_method(rb_cMIDINoteMessage, "velocity", midi_note_message_velocity, 0);
    rb_define_method(rb_cMIDINoteMessage, "release_velocity", midi_note_message_release_velocity, 0);
    rb_define_method(rb_cMIDINoteMessage, "duration", midi_note_message_duration, 0);
    
    /* AudioToolbox::MIDIChannelMessage */
    rb_cMIDIChannelMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIChannelMessage", rb_cObject);
    rb_define_singleton_method(rb_cMIDIChannelMessage, "new", midi_channel_message_new, 1);
    rb_define_method(rb_cMIDIChannelMessage, "initialize", midi_channel_message_init, 1);
    rb_define_method(rb_cMIDIChannelMessage, "status", midi_channel_message_status, 0);
    rb_define_method(rb_cMIDIChannelMessage, "data1", midi_channel_message_data1, 0);
    rb_define_method(rb_cMIDIChannelMessage, "data2", midi_channel_message_data2, 0);
}
