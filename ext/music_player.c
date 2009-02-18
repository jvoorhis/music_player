#include <ruby.h>

#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/MIDIServices.h>

/* Type defns */

static VALUE rb_mCoreMIDI = Qnil;
static VALUE rb_mAudioToolbox = Qnil;
static VALUE rb_cMusicPlayer = Qnil;
static VALUE rb_cMusicSequence = Qnil;
static VALUE rb_cMusicTrack = Qnil;
static VALUE rb_cMIDINoteMessage = Qnil;

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
player_free (MusicPlayer player)
{
    OSStatus err;
    require_noerr( err = DisposeMusicPlayer(player), fail );
    
    fail:
    rb_warning("DisposeMusicPlayer() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_new (VALUE class)
{
    OSStatus err;
    MusicPlayer *ptrPlayer = ALLOC(MusicPlayer);
    require_noerr( err = NewMusicPlayer(ptrPlayer), fail );
    
    VALUE player = Data_Wrap_Struct(class, 0, player_free, ptrPlayer);
    rb_obj_call_init(player, 0, 0);
    return player;

    fail:
    rb_raise(rb_eRuntimeError, "NewMusicPlayer() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_is_playing (VALUE self)
{
    MusicPlayer *ptrPlayer;
    Data_Get_Struct(self, MusicPlayer, ptrPlayer);
    
    Boolean playing;
    OSStatus err;
    require_noerr( err = MusicPlayerIsPlaying(*ptrPlayer, &playing), fail );
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
player_set_sequence (VALUE self, VALUE seq)
{
    MusicPlayer *ptrPlayer;
    Data_Get_Struct(self, MusicPlayer, ptrPlayer);
    
    MusicSequence *ptrSeq;
    Data_Get_Struct(seq, MusicSequence, ptrSeq);
    
    rb_iv_set(self, "@sequence", seq);

    OSStatus err;
    require_noerr( err = MusicPlayerSetSequence(*ptrPlayer, *ptrSeq), fail );
    return seq;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerSetSequence() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_start (VALUE self)
{
    MusicPlayer *ptrPlayer;
    Data_Get_Struct(self, MusicPlayer, ptrPlayer);
    
    OSStatus err;
    require_noerr( err = MusicPlayerStart(*ptrPlayer), fail );
    
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerStart() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
player_stop (VALUE self)
{
    MusicPlayer *ptrPlayer;
    Data_Get_Struct(self, MusicPlayer, ptrPlayer);

    OSStatus err;
    require_noerr( err = MusicPlayerStop(*ptrPlayer), fail );
    
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerStop() failed with OSStatus %i.", (SInt32) err);
}

/* Sequence defns */

static VALUE
sequence_new (VALUE class)
{
    MusicSequence *ptrSeq = ALLOC(MusicSequence);
    OSStatus err;
    require_noerr( err = NewMusicSequence(ptrSeq), fail );
    VALUE seq = Data_Wrap_Struct(class, 0, 0, ptrSeq);
    rb_obj_call_init(seq, 0, 0);
    return seq;
    
    fail:
    rb_raise(rb_eRuntimeError, "NewMusicSequence() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
sequence_set_midi_endpoint (VALUE self, VALUE endpoint_ref)
{
    if (NIL_P(endpoint_ref)) { return Qnil; }
    MusicSequence *ptrSeq;
    Data_Get_Struct(self, MusicSequence, ptrSeq);
    
    UInt32 ref = NUM2ULONG(endpoint_ref);
    OSStatus err;
    require_noerr( err = MusicSequenceSetMIDIEndpoint(*ptrSeq, (MIDIEndpointRef) ref), fail);
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceSetMIDIEndpoint() failed with OSStatus %i.", (SInt32) err);
}

/* Track defns */

static VALUE
track_init (VALUE self, VALUE seq)
{
    rb_iv_set(self, "@sequence", seq);
    return self;
}

static VALUE
track_new (VALUE class, VALUE seq)
{
    MusicSequence *ptrSeq;
    Data_Get_Struct(seq, MusicSequence, ptrSeq);
    
    MusicTrack *ptrTrack = ALLOC(MusicTrack);
    OSStatus err;
    require_noerr( err = MusicSequenceNewTrack(*ptrSeq, ptrTrack), fail );
    
    VALUE track = Data_Wrap_Struct(class, 0, 0, ptrTrack);
    
    VALUE argv[1];
    argv[0] = seq;
    rb_obj_call_init(track, 1, argv);
    return track;

    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceNewTrack() failed with OSStatus %i.", (SInt32) err);
}

static VALUE
track_add_midi_note_message (VALUE self, VALUE at, VALUE msg)
{
    MusicTrack *ptrTrack;
    Data_Get_Struct(self, MusicTrack, ptrTrack);
    
    MIDINoteMessage *ptrMsg;
    Data_Get_Struct(msg, MIDINoteMessage, ptrMsg);
    
    MusicTimeStamp ts = (MusicTimeStamp) NUM2DBL(at);
    OSStatus err;
    require_noerr( err = MusicTrackNewMIDINoteEvent(*ptrTrack, ts, ptrMsg), fail );
    
    return Qnil;

    fail:
    rb_raise(rb_eRuntimeError, "MusicTrackNewMIDINoteEvent() failed with OSStatus %i.", (SInt32) err);
}

/* MIDINoteMessage */

static VALUE
midi_note_message_new (VALUE class, VALUE opts)
{
    if (T_HASH != TYPE(opts)) {
        rb_raise(rb_eTypeError, "Expected opts to be a Hash.");
    }
    
    MIDINoteMessage *ptrMsg = ALLOC(MIDINoteMessage);
    VALUE optChn, optNote, optVel, optRelVel, optDur;
    
    optChn = rb_hash_aref(opts, ID2SYM(rb_intern("channel")));
    ptrMsg->channel = (UInt8) T_FIXNUM == TYPE(optChn) ? FIX2UINT(optChn) : 1;
    
    optNote = rb_hash_aref(opts, ID2SYM(rb_intern("pitch"))); // FIXME no default pitch.
    ptrMsg->note = (UInt8) T_FIXNUM == TYPE(optNote) ? FIX2UINT(optNote) : 60;
    
    optVel = rb_hash_aref(opts, ID2SYM(rb_intern("velocity")));
    ptrMsg->velocity = (UInt8) T_FIXNUM == TYPE(optVel) ? FIX2UINT(optVel) : 64;
    
    optRelVel = rb_hash_aref(opts, ID2SYM(rb_intern("release_velocity")));
    ptrMsg->releaseVelocity = (UInt8) T_FIXNUM == TYPE(optRelVel) ? FIX2UINT(optRelVel) : 0;
    
    optDur = rb_hash_aref(opts, ID2SYM(rb_intern("duration")));
    ptrMsg->duration = (MusicTimeStamp) (T_FLOAT == TYPE(optDur) || T_FIXNUM == TYPE(optDur)) ? NUM2DBL(optDur) : 1.0;
    
    VALUE msg = Data_Wrap_Struct(class, 0, 0, ptrMsg);
    rb_obj_call_init(msg, 0, 0);
    return msg;
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
}
