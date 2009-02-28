/*
 * Copyright (c) 2009 Jeremy Voorhis <jvoorhis@gmail.com>
 */

#include <ruby.h>
#include "util.h"
#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/MIDIServices.h>

/* Ruby type decls */

static VALUE rb_mCoreMIDI = Qnil;
static VALUE rb_mAudioToolbox = Qnil;
static VALUE rb_cMusicPlayer = Qnil;
static VALUE rb_cMusicSequence = Qnil;
static VALUE rb_cMusicTrack = Qnil;
static VALUE rb_cMusicTrackCollection = Qnil;
static VALUE rb_cMIDINoteMessage = Qnil;
static VALUE rb_cMIDIChannelMessage = Qnil;
static VALUE rb_cExtendedTempoEvent = Qnil;

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
    free(player);
    return;
    
    fail:
    rb_warning("DisposeMusicPlayer() failed with OSStatus %i.", (int) err);
}

static VALUE
player_alloc (VALUE class)
{
    MusicPlayer *player;
    return Data_Make_Struct(rb_cMusicPlayer, MusicPlayer, 0, player_free, player);
}

static VALUE
player_init (VALUE self)
{
    MusicPlayer *player;
    OSStatus err;
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = NewMusicPlayer(player), fail );
    return self;
    
    fail:
    rb_raise(rb_eRuntimeError, "NewMusicPlayer() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicPlayerIsPlaying() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicPlayerSetSequence() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicPlayerStart() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicPlayerStop() failed with OSStatus %i.", (int) err);
}

static VALUE
player_get_time (VALUE self)
{
    MusicPlayer *player;
    MusicTimeStamp ts;
    OSStatus err;
    
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerGetTime(*player, &ts), fail );
    return rb_float_new((Float64) ts);
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerGetTime() failed with OSStatus %i.", (int) err);
}

static VALUE
player_set_time (VALUE self, VALUE rb_ts)
{
    if (!PRIM_NUM_P(rb_ts))
        rb_raise(rb_eArgError, "Expected argument to be a number.");
    
    MusicPlayer *player;
    MusicTimeStamp ts;
    OSStatus err;
    
    ts = rb_num2dbl(rb_ts);
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerSetTime(*player, ts), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerSetTime() failed with OSStatus %i.", (int) err);
}

static VALUE
player_get_play_rate_scalar (VALUE self)
{
    MusicPlayer *player;
    Float64 scalar;
    OSStatus err;
    
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerGetPlayRateScalar(*player, &scalar), fail );
    return rb_float_new(scalar);

    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerGetPlayRateScalar() failed with OSStatus %i.", (int) err);
}

static VALUE
player_set_play_rate_scalar (VALUE self, VALUE rb_scalar)
{
    if (!PRIM_NUM_P(rb_scalar))
        rb_raise(rb_eArgError, "Expected scalar to be a number.");
    
    MusicPlayer *player;
    Float64 scalar;
    OSStatus err;

    scalar = NUM2DBL(rb_scalar);
    Data_Get_Struct(self, MusicPlayer, player);
    require_noerr( err = MusicPlayerSetPlayRateScalar(*player, scalar), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicPlayerSetPlayRateScalar() failed with %i.", (int) err);
}

/* Sequence defns */

static void
sequence_free (MusicSequence *seq)
{
    OSStatus err;
    require_noerr( err = DisposeMusicSequence(*seq), fail );
    free(seq);
    return;
    
    fail:
    rb_warning("DisposeMusicSequence() failed with %i.", (int) err);
}

static VALUE
sequence_alloc (VALUE class)
{
  MusicSequence *seq;
  return Data_Make_Struct(rb_cMusicSequence, MusicSequence, 0, sequence_free, seq);
}

static VALUE
sequence_init (VALUE self)
{
    MusicSequence *seq;
    OSStatus err;
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = NewMusicSequence(seq), fail );
    return self;
    
    fail:
    rb_raise(rb_eRuntimeError, "NewMusicSequence() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicSequenceSetMIDIEndpoint() failed with OSStatus %i.", (int) err);
}

static VALUE
sequence_get_type (VALUE self)
{
    MusicSequence *seq;
    MusicSequenceType type;
    OSStatus err;
    
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = MusicSequenceGetSequenceType(*seq, &type), fail );
    
    VALUE rb_type = Qnil;
    switch (type) {
    case kMusicSequenceType_Beats:
        rb_type = CSTR2SYM("beat");
        break;
    case kMusicSequenceType_Seconds:
        rb_type = CSTR2SYM("secs");
        break;
    case kMusicSequenceType_Samples:
        rb_type = CSTR2SYM("samp");
        break;
    default:
        rb_raise(rb_eRuntimeError, "Unrecognized sequence type.");
    }
    return rb_type;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceGetSequenceType() failed with OSStatus %i.", (int) err);
}

static VALUE
sequence_set_type (VALUE self, VALUE rb_type)
{
    MusicSequence *seq;
    MusicSequenceType type;
    if (rb_type == CSTR2SYM("beat"))
        type = kMusicSequenceType_Beats;
    else if (rb_type == CSTR2SYM("secs"))
        type = kMusicSequenceType_Seconds;
    else if (rb_type == CSTR2SYM("samp"))
        type = kMusicSequenceType_Samples;
    else
        rb_raise(rb_eArgError, "Expected :type to be one of :beat, :secs, :samp.");
    
    Data_Get_Struct(self, MusicSequence, seq);
    OSStatus err;
    require_noerr( err = MusicSequenceSetSequenceType(*seq, type), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceGetSequenceType() failed with OSStatus %i.", (int) err);
}

static VALUE
sequence_save (VALUE self, VALUE rb_path)
{
    CFURLRef url = PATH2CFURL(StringValue(rb_path));
    MusicSequence *seq;
    OSStatus err;
    
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = MusicSequenceFileCreate(*seq, url, kMusicSequenceFile_MIDIType, kMusicSequenceFileFlags_EraseFile, 0), fail );
    CFRelease(url);
    
    return Qnil;
    
    fail:
    CFRelease(url);
    rb_raise(rb_eRuntimeError, "MusicSequenceFileCreate() failed with OSStatus %i.", (int) err);
}

static VALUE
sequence_load (VALUE self, VALUE rb_path)
{
    CFURLRef url = PATH2CFURL(StringValue(rb_path));
    MusicSequence *seq;
    OSStatus err;
    
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = MusicSequenceFileLoad(*seq, url, kMusicSequenceFile_MIDIType, kMusicSequenceLoadSMF_ChannelsToTracks), fail );
    CFRelease(url);
    
    return Qnil;
    
    fail:
    CFRelease(url);
    rb_raise(rb_eRuntimeError, "MusicSequenceFileLoad() failed with OSStatus %i.", (int) err);
}

/* Track defns */

void
track_free (MusicTrack *track)
{
    free(track);
}

static VALUE
track_init (VALUE self, VALUE rb_seq)
{
    rb_iv_set(self, "@sequence", rb_seq);
    return self;
}

static VALUE
track_internal_new (VALUE rb_seq, MusicTrack *track)
{
    VALUE rb_track, argv[1];
    rb_track = Data_Wrap_Struct(rb_cMusicTrack, 0, track_free, track);
    argv[0] = rb_seq;
    rb_obj_call_init(rb_track, 1, argv);
    return rb_track;
}

static VALUE
track_new (VALUE class, VALUE rb_seq)
{
    MusicSequence *seq;
    MusicTrack *track;
    OSStatus err;
    VALUE rb_track, argv[1];
    
    Data_Get_Struct(rb_seq, MusicSequence, seq);
    rb_track = Data_Make_Struct(rb_cMusicTrack, MusicTrack, 0, track_free, track);
    require_noerr( err = MusicSequenceNewTrack(*seq, track), fail );
    argv[0] = rb_seq;
    rb_obj_call_init(rb_track, 1, argv);
    return rb_track;

    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceNewTrack() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicTrackNewMIDINoteEvent() failed with OSStatus %i.", (int) err);
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
    rb_raise(rb_eRuntimeError, "MusicTrackNewMIDIChannelEvent() failed with OSStatus %i.", (int) err);
}

static VALUE
track_add_extended_tempo_event (VALUE self, VALUE rb_at, VALUE rb_bpm)
{
    MusicTrack *track;
    MusicTimeStamp ts;
    Float64 bpm;
    OSStatus err;
    
    Data_Get_Struct(self, MusicTrack, track);
    
    if (PRIM_NUM_P(rb_at))
        ts = NUM2DBL(rb_at);
    else
        rb_raise(rb_eArgError, "Expected first arg to be a number.");
    
    if (PRIM_NUM_P(rb_bpm))
        bpm = NUM2DBL(rb_bpm);
    else
        rb_raise(rb_eArgError, "Expected second arg to be a number.");
    
    require_noerr( err = MusicTrackNewExtendedTempoEvent(*track, ts, bpm), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicTrackNewExtendedTempoEvent() failed with OSStatus %i.", (int) err);
}

/* MusicSequence#Tracks proxy defns */

MusicSequence*
tracks_get_seq (VALUE rb_tracks)
{
    MusicSequence *seq;
    VALUE rb_seq = rb_iv_get(rb_tracks, "@sequence");
    Data_Get_Struct(rb_seq, MusicSequence, seq);
    return seq;
}

static VALUE
tracks_size (VALUE self)
{
    MusicSequence *seq = tracks_get_seq(self);
    UInt32 track_count;
    OSStatus err;
    
    require_noerr( err = MusicSequenceGetTrackCount(*seq, &track_count), fail );
    return UINT2NUM(track_count);
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceGetTrackCount() failed with OSStatus %i.", (int) err);
}

static VALUE
tracks_get_ind_track_internal (VALUE self, VALUE rb_key)
{
    if (!FIXNUM_P(rb_key)) rb_raise(rb_eArgError, "Expected key to be a Fixnum.");
    MusicSequence *seq = tracks_get_seq(self);
    MusicTrack *track = ALLOC(MusicTrack);
    VALUE rb_seq = rb_iv_get(self, "@sequence");
    OSStatus err;
    
    require_noerr( err = MusicSequenceGetIndTrack(*seq, FIX2INT(rb_key), track), fail );
    return track_internal_new(rb_seq, track);
    
    fail:
    if (err == kAudioToolboxErr_TrackIndexError)
        rb_raise(rb_eRangeError, "Index is out-of-bounds.");
    else
        rb_raise(rb_eRuntimeError, "MusicSequenceGetIndTrack() failed with OSStatus %i.", (int) err);
}

static VALUE
tracks_index (VALUE self, VALUE rb_track)
{
    if (rb_cMusicTrack != rb_class_of(rb_track))
        rb_raise(rb_eArgError, "Expected arg to be a MusicTrack.");
    
    MusicSequence *seq = tracks_get_seq(self);
    MusicTrack *track;
    UInt32 i;
    OSStatus err;
    
    Data_Get_Struct(rb_track, MusicTrack, track);
    require_noerr( err = MusicSequenceGetTrackIndex(*seq, *track, &i), fail );
    return UINT2NUM(i);
    
    fail:
    rb_raise(rb_eRangeError, "MusicSequenceGetTrackIndex() failed with OSStatus %i.", (int) err);
}

static VALUE
tracks_tempo_internal (VALUE self)
{
    MusicSequence *seq = tracks_get_seq(self);
    VALUE rb_seq = rb_iv_get(self, "@sequence");
    MusicTrack *track = ALLOC(MusicTrack);
    OSStatus err;
    
    require_noerr( err = MusicSequenceGetTempoTrack(*seq, track), fail );
    return track_internal_new(rb_seq, track);
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceGetTempoTrack() failed with OSStatus %i.", (int) err);
}

static VALUE
tracks_delete_internal (VALUE self, VALUE rb_track)
{
    MusicSequence *seq = tracks_get_seq(self);
    MusicTrack *track;
    OSStatus err;
    
    Data_Get_Struct(rb_track, MusicTrack, track);
    require_noerr( err = MusicSequenceDisposeTrack(*seq, *track), fail );
    return Qnil;
    
    fail:
    rb_raise(rb_eRuntimeError, "MusicSequenceDisposeTrack() failed with OSStatus %i.", (int) err);
}

/* MIDINoteMessage */

static void
midi_note_message_free (MIDINoteMessage *msg)
{
    free(msg);
}

static VALUE
midi_note_message_init (VALUE self, VALUE rb_opts)
{
    Check_Type(rb_opts, T_HASH);
    MIDINoteMessage *msg;
    VALUE rb_chn, rb_note, rb_vel, rb_rel_vel, rb_dur;

    Data_Get_Struct(self, MIDINoteMessage, msg);

    rb_chn = rb_hash_aref(rb_opts, CSTR2SYM("channel"));
    msg->channel = FIXNUM_P(rb_chn) ? FIX2UINT(rb_chn) : 1;
    
    rb_note = rb_hash_aref(rb_opts, CSTR2SYM("note"));
    if (FIXNUM_P(rb_note))
        msg->note = FIX2UINT(rb_note);
    else
        rb_raise(rb_eArgError, ":note is required.");
    
    rb_vel = rb_hash_aref(rb_opts, CSTR2SYM("velocity"));
    msg->velocity = FIXNUM_P(rb_vel) ? FIX2UINT(rb_vel) : 64;
    
    rb_rel_vel = rb_hash_aref(rb_opts, CSTR2SYM("release_velocity"));
    msg->releaseVelocity = FIXNUM_P(rb_rel_vel) ? FIX2UINT(rb_rel_vel) : 0;
    
    rb_dur = rb_hash_aref(rb_opts, CSTR2SYM("duration"));
    msg->duration = (MusicTimeStamp) (PRIM_NUM_P(rb_dur)) ? NUM2DBL(rb_dur) : 1.0;
    
    return self;
}

static VALUE
midi_note_message_alloc (VALUE class)
{
    MIDINoteMessage *msg;
    return Data_Make_Struct(class, MIDINoteMessage, 0, midi_note_message_free, msg);
}

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

/* MIDIChannelMessage */

static void
midi_channel_message_free (MIDIChannelMessage *msg)
{
    free(msg);
}

static VALUE
midi_channel_message_init (VALUE self, VALUE rb_opts)
{
    Check_Type(rb_opts, T_HASH);
    MIDIChannelMessage *msg;
    VALUE rb_status, rb_data1, rb_data2;
    
    Data_Get_Struct(self, MIDIChannelMessage, msg);
    
    rb_status = rb_hash_aref(rb_opts, CSTR2SYM("status"));
    if (!FIXNUM_P(rb_status))
        rb_raise(rb_eArgError, ":status is required.");
    else
        msg->status = NUM2DBL(rb_status);
    
    rb_data1 = rb_hash_aref(rb_opts, CSTR2SYM("data1"));
    if (!NIL_P(rb_data1)) msg->data1 = (UInt8) FIX2INT(rb_data1);
    
    rb_data2 = rb_hash_aref(rb_opts, CSTR2SYM("data2"));
    if (!NIL_P(rb_data2)) msg->data2 = (UInt8) FIX2INT(rb_data2);
    
    return self;
}

static VALUE
midi_channel_message_alloc (VALUE class)
{
    MIDIChannelMessage *msg;
    return Data_Make_Struct(class, MIDIChannelMessage, 0, midi_channel_message_free, msg);
}

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
    rb_define_alloc_func(rb_cMusicPlayer, player_alloc);
    rb_define_method(rb_cMusicPlayer, "initialize", player_init, 0);
    rb_define_method(rb_cMusicPlayer, "playing?", player_is_playing, 0);
    rb_define_method(rb_cMusicPlayer, "sequence", player_get_sequence, 0);
    rb_define_method(rb_cMusicPlayer, "sequence=", player_set_sequence, 1);
    rb_define_method(rb_cMusicPlayer, "start", player_start, 0);
    rb_define_method(rb_cMusicPlayer, "stop", player_stop, 0);
    rb_define_method(rb_cMusicPlayer, "time", player_get_time, 0);
    rb_define_method(rb_cMusicPlayer, "time=", player_set_time, 1);
    rb_define_method(rb_cMusicPlayer, "play_rate_scalar", player_get_play_rate_scalar, 0);
    rb_define_method(rb_cMusicPlayer, "play_rate_scalar=", player_set_play_rate_scalar, 1);
    
    /* AudioToolbox::MusicSequence */
    rb_cMusicSequence = rb_define_class_under(rb_mAudioToolbox, "MusicSequence", rb_cObject);
    rb_define_alloc_func(rb_cMusicSequence, sequence_alloc);
    rb_define_method(rb_cMusicSequence, "initialize", sequence_init, 0);
    rb_define_private_method(rb_cMusicSequence, "load_internal", sequence_load, 1);
    rb_define_method(rb_cMusicSequence, "midi_endpoint=", sequence_set_midi_endpoint, 1);
    rb_define_method(rb_cMusicSequence, "type", sequence_get_type, 0);
    rb_define_method(rb_cMusicSequence, "type=", sequence_set_type, 1);
    rb_define_method(rb_cMusicSequence, "save", sequence_save, 1);
    
    /* AudioToolbox::MusicTrack */
    rb_cMusicTrack = rb_define_class_under(rb_mAudioToolbox, "MusicTrack", rb_cObject);
    rb_define_singleton_method(rb_cMusicTrack, "new", track_new, 1);
    rb_define_method(rb_cMusicTrack, "initialize", track_init, 1);
    rb_define_method(rb_cMusicTrack, "add_midi_note_message", track_add_midi_note_message, 2);
    rb_define_method(rb_cMusicTrack, "add_midi_channel_message", track_add_midi_channel_message, 2);
    rb_define_method(rb_cMusicTrack, "add_extended_tempo_event", track_add_extended_tempo_event, 2);
    
    /* AudioToolbox::MusicSequence#tracks proxy */
    rb_cMusicTrackCollection = rb_define_class_under(rb_mAudioToolbox, "MusicTrackCollection", rb_cObject);
    rb_define_method(rb_cMusicTrackCollection, "size", tracks_size, 0);
    rb_define_method(rb_cMusicTrackCollection, "index", tracks_index, 1);
    rb_define_private_method(rb_cMusicTrackCollection, "delete_internal", tracks_delete_internal, 1);
    rb_define_private_method(rb_cMusicTrackCollection, "tempo_internal", tracks_tempo_internal, 0);
    rb_define_private_method(rb_cMusicTrackCollection, "ind_internal", tracks_get_ind_track_internal, 1);
    
    /* AudioToolbox::MIDINoteMessage */
    rb_cMIDINoteMessage = rb_define_class_under(rb_mAudioToolbox, "MIDINoteMessage", rb_cObject);
    rb_define_alloc_func(rb_cMIDINoteMessage, midi_note_message_alloc);
    rb_define_method(rb_cMIDINoteMessage, "initialize", midi_note_message_init, 1);
    rb_define_method(rb_cMIDINoteMessage, "channel", midi_note_message_channel, 0);
    rb_define_method(rb_cMIDINoteMessage, "note", midi_note_message_note, 0);
    rb_define_method(rb_cMIDINoteMessage, "velocity", midi_note_message_velocity, 0);
    rb_define_method(rb_cMIDINoteMessage, "release_velocity", midi_note_message_release_velocity, 0);
    rb_define_method(rb_cMIDINoteMessage, "duration", midi_note_message_duration, 0);
    
    /* AudioToolbox::MIDIChannelMessage */
    rb_cMIDIChannelMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIChannelMessage", rb_cObject);
    rb_define_alloc_func(rb_cMIDIChannelMessage, midi_channel_message_alloc);
    rb_define_method(rb_cMIDIChannelMessage, "initialize", midi_channel_message_init, 1);
    rb_define_method(rb_cMIDIChannelMessage, "status", midi_channel_message_status, 0);
    rb_define_method(rb_cMIDIChannelMessage, "data1", midi_channel_message_data1, 0);
    rb_define_method(rb_cMIDIChannelMessage, "data2", midi_channel_message_data2, 0);
    
    /* AudioToolbox::ExtendedTempoEvent */
    rb_cExtendedTempoEvent = rb_define_class_under(rb_mAudioToolbox, "ExtendedTempoEvent", rb_cObject);
}
