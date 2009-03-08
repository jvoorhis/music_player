/*
 * Copyright (c) 2009 Jeremy Voorhis <jvoorhis@gmail.com>
 */

#include <ruby.h>
#include "util.h"
#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/MIDIServices.h>

/* Ruby type decls */

static VALUE rb_mCoreMIDI;

static VALUE rb_mAudioToolbox;

static VALUE rb_eTrackNotFound;
static VALUE rb_eEndOfTrack;
static VALUE rb_eStartOfTrack;
static VALUE rb_eNoSequence;
static VALUE rb_eIllegalTrackDestination;

static VALUE rb_cMusicPlayer;
static VALUE rb_cMusicSequence;
static VALUE rb_cMusicTrack;
static VALUE rb_cMusicTrackCollection;
static VALUE rb_cMIDINoteMessage;
static VALUE rb_cMIDIChannelMessage;
static VALUE rb_cMIDIKeyPressureMessage;
static VALUE rb_cMIDIControlChangeMessage;
static VALUE rb_cMIDIProgramChangeMessage;
static VALUE rb_cMIDIChannelPressureMessage;
static VALUE rb_cMIDIPitchBendMessage;
static VALUE rb_cExtendedTempoEvent;
static VALUE rb_cMusicEventIterator;

/* Ruby symbols */
static VALUE rb_sBeat;
static VALUE rb_sBpm;
static VALUE rb_sChannel;
static VALUE rb_sData1;
static VALUE rb_sData2;
static VALUE rb_sDuration;
static VALUE rb_sLength;
static VALUE rb_sLoopInfo;
static VALUE rb_sMute;
static VALUE rb_sNote;
static VALUE rb_sNumber;
static VALUE rb_sPressure;
static VALUE rb_sProgram;
static VALUE rb_sReleaseVelocity;
static VALUE rb_sSamp;
static VALUE rb_sSecs;
static VALUE rb_sSolo;
static VALUE rb_sStatus;
static VALUE rb_sValue;
static VALUE rb_sVelocity;

/* Utils */

#define RAISE_OSSTATUS(error,what) \
    if (error == kAudioToolboxErr_TrackIndexError) {\
        rb_raise(rb_eRangeError, "Index is out of range.");\
    } else if (error == kAudioToolboxErr_TrackNotFound) {\
        rb_raise(rb_eTrackNotFound, "Track not found.");\
    } else if (error == kAudioToolboxErr_EndOfTrack) {\
        rb_raise(rb_eEndOfTrack, "Reached end of track.");\
    } else if (error == kAudioToolboxErr_StartOfTrack) {\
        rb_raise(rb_eStartOfTrack, "Reached start of track.");\
    } else if (error == kAudioToolboxErr_IllegalTrackDestination) {\
        rb_raise(rb_eIllegalTrackDestination, "Illegal track destination.");\
    } else if (error == kAudioToolboxErr_NoSequence) {\
        rb_raise(rb_eNoSequence, "No sequence was given.");\
    } else {\
        rb_raise(rb_eRuntimeError, "%s failed with OSStatus %i.", what, (int)error);\
    }

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
    if (player) {
        require_noerr( err = DisposeMusicPlayer(*player), fail );
        free(player);
    }
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
    RAISE_OSSTATUS(err, "NewMusicPlayer()");
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
    RAISE_OSSTATUS(err, "MusicPlayerIsPlaying()");
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
    RAISE_OSSTATUS(err, "MusicPlayerSetSequence()");
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
    RAISE_OSSTATUS(err, "MusicPlayerStart()");
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
    RAISE_OSSTATUS(err, "MusicPlayerStop()");
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
    RAISE_OSSTATUS(err, "MusicPlayerGetTime()");
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
    RAISE_OSSTATUS(err, "MusicPlayerSetTime()");
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
    RAISE_OSSTATUS(err, "MusicPlayerGetPlayRateScalar()");
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
    RAISE_OSSTATUS(err, "MusicPlayerSetPlayRateScalar()");
}

/* Sequence defns */

static void
sequence_free (MusicSequence *seq)
{
    OSStatus err;
    if (seq) {
        require_noerr( err = DisposeMusicSequence(*seq), fail );
        free(seq);
    }
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
    rb_iv_set(self, "@tracks",
              rb_funcall(rb_cMusicTrackCollection, rb_intern("new"), 1, self));
    
    return self;
    
    fail:
    RAISE_OSSTATUS(err, "NewMusicSequence()");
}

static VALUE
sequence_set_midi_endpoint (VALUE self, VALUE rb_endpoint_ref)
{
    MusicSequence *seq;
    UInt32 ref = NUM2ULONG(rb_funcall(rb_mKernel, rb_intern("Integer"), 1, rb_endpoint_ref));
    OSStatus err;
    
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = MusicSequenceSetMIDIEndpoint(*seq, (MIDIEndpointRef) ref), fail);
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicSequenceSetMIDIEndpoint()");
}

static VALUE
sequence_get_type (VALUE self)
{
    MusicSequence *seq;
    MusicSequenceType type;
    OSStatus err;
    
    Data_Get_Struct(self, MusicSequence, seq);
    require_noerr( err = MusicSequenceGetSequenceType(*seq, &type), fail );
    
    switch (type) {
    case kMusicSequenceType_Beats:
        return rb_sBeat;
    case kMusicSequenceType_Seconds:
        return rb_sSecs;
    case kMusicSequenceType_Samples:
        return rb_sSamp;
    default:
        rb_raise(rb_eRuntimeError, "Unrecognized sequence type.");
    }
    
    fail:
    RAISE_OSSTATUS(err, "MusicSequenceGetSequenceType()");
}

static VALUE
sequence_set_type (VALUE self, VALUE rb_type)
{
    MusicSequence *seq;
    MusicSequenceType type;
    if (rb_type == rb_sBeat)
        type = kMusicSequenceType_Beats;
    else if (rb_type == rb_sSecs)
        type = kMusicSequenceType_Seconds;
    else if (rb_type == rb_sSamp)
        type = kMusicSequenceType_Samples;
    else
        rb_raise(rb_eArgError, "Expected :type to be one of :beat, :secs, :samp.");
    
    Data_Get_Struct(self, MusicSequence, seq);
    OSStatus err;
    require_noerr( err = MusicSequenceSetSequenceType(*seq, type), fail );
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicSequenceSetSequenceType()");
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
    RAISE_OSSTATUS(err, "MusicSequenceFileCreate()");
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
    RAISE_OSSTATUS(err, "MusicSequenceFileLoad()");
}

/* Track defns */

static void
track_free (MusicTrack *track)
{
    if(track) free(track);
}

static VALUE
track_init (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_seq, rb_options;
    rb_scan_args(argc, argv, "11", &rb_seq, &rb_options);
    
    rb_iv_set(self, "@sequence", rb_seq);
    
    if (T_HASH == TYPE(rb_options)) {
        VALUE loop_info = rb_hash_aref(rb_options, rb_sLoopInfo),
              mute      = rb_hash_aref(rb_options, rb_sMute),
              solo      = rb_hash_aref(rb_options, rb_sSolo),
              length    = rb_hash_aref(rb_options, rb_sLength);
        
        if (!NIL_P(loop_info))
            rb_funcall(self, rb_intern("loop_info="), 1, loop_info);
        if (!NIL_P(mute))
            rb_funcall(self, rb_intern("mute="), 1, mute);
        if (!NIL_P(solo))
            rb_funcall(self, rb_intern("solo="), 1, solo);
        if (!NIL_P(length))
            rb_funcall(self, rb_intern("length="), 1, length);
    }
    
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
track_new (int argc, VALUE *argv, VALUE class)
{
    VALUE rb_seq, rb_options, rb_track, init_argv[2];
    MusicSequence *seq;
    MusicTrack *track;
    OSStatus err;
    
    rb_scan_args(argc, argv, "11", &rb_seq, &rb_options);
    Data_Get_Struct(rb_seq, MusicSequence, seq);
    
    rb_track = Data_Make_Struct(rb_cMusicTrack, MusicTrack, 0, track_free, track);
    require_noerr( err = MusicSequenceNewTrack(*seq, track), fail );
    init_argv[0] = rb_seq;
    init_argv[1] = rb_options;
    rb_obj_call_init(rb_track, 2, init_argv);
    return rb_track;

    fail:
    RAISE_OSSTATUS(err, "MusicSequenceNewTrack()");
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
    RAISE_OSSTATUS(err, "MusicTrackNewMIDINoteEvent()");
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
    RAISE_OSSTATUS(err, "MusicTrackNewMIDIChannelEvent()");
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
    RAISE_OSSTATUS(err, "MusicTrackNewExtendedTempoEvent()");
}

static VALUE
track_get_loop_info (VALUE self)
{
    MusicTrack *track;
    UInt32 sz;
    MusicTrackLoopInfo loop_info;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    require_noerr( err = MusicTrackGetProperty(*track, kSequenceTrackProperty_LoopInfo, &loop_info, &sz), fail );

    if (sz == sizeof(MusicTrackLoopInfo)) {
        VALUE rb_loop_info = rb_hash_new();
        rb_hash_aset(rb_loop_info, rb_sDuration, rb_float_new(loop_info.loopDuration));
        rb_hash_aset(rb_loop_info, rb_sNumber, INT2NUM(loop_info.numberOfLoops));
        return rb_loop_info;
    } else {
        return Qnil;
    }
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

static VALUE
track_set_loop_info (VALUE self, VALUE rb_loop_info)
{
    Check_Type(rb_loop_info, T_HASH);
    MusicTrack *track;
    MusicTrackLoopInfo loop_info;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    loop_info.loopDuration = NUM2DBL(rb_hash_aref(rb_loop_info, rb_sDuration));
    loop_info.numberOfLoops = NUM2DBL(rb_hash_aref(rb_loop_info, rb_sNumber));
    
    require_noerr(
        err = MusicTrackSetProperty(*track, kSequenceTrackProperty_LoopInfo,
                                    &loop_info, sizeof(MusicTrackLoopInfo)),
        fail);
    
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackSetProperty()");
}

static VALUE
track_get_offset (VALUE self)
{
    MusicTrack *track;
    UInt32 sz;
    MusicTimeStamp offset;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    require_noerr( err = MusicTrackGetProperty(*track, kSequenceTrackProperty_OffsetTime, &offset, &sz), fail );
    
    if (sz == sizeof(MusicTimeStamp))
        return rb_float_new(offset);
    else
        return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

static VALUE
track_set_offset (VALUE self, VALUE rb_offset)
{
    if (!PRIM_NUM_P(rb_offset))
        rb_raise(rb_eTypeError, "Expected offset to be a number.");
    MusicTrack *track;
    MusicTimeStamp offset = NUM2DBL(rb_offset);
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    
    require_noerr(
        err = MusicTrackSetProperty(*track, kSequenceTrackProperty_OffsetTime,
                                    &offset, sizeof(MusicTimeStamp)),
        fail);
    
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackSetProperty()");
}

static VALUE
track_get_mute (VALUE self)
{
    MusicTrack *track;
    UInt32 sz;
    Boolean status;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    require_noerr( err = MusicTrackGetProperty(*track, kSequenceTrackProperty_MuteStatus, &status, &sz), fail );
    
    if (sz == sizeof(Boolean))
        return status ? Qtrue : Qfalse;
    else
        return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

static VALUE
track_set_mute (VALUE self, VALUE rb_status)
{
    MusicTrack *track;
    Boolean status = RTEST(rb_status);
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    
    require_noerr(
        err = MusicTrackSetProperty(*track, kSequenceTrackProperty_MuteStatus,
                                    &status, sizeof(Boolean)),
        fail);
    
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackSetProperty()");
}

static VALUE
track_get_solo (VALUE self)
{
    MusicTrack *track;
    UInt32 sz;
    Boolean status;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    require_noerr(
        err = MusicTrackGetProperty(*track, kSequenceTrackProperty_SoloStatus,
                                    &status, &sz),
        fail );
    
    if (sz == sizeof(Boolean))
        return status ? Qtrue : Qfalse;
    else
        return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

static VALUE
track_set_solo (VALUE self, VALUE rb_status)
{
    MusicTrack *track;
    Boolean status = RTEST(rb_status);
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    
    require_noerr(
        err = MusicTrackSetProperty(*track, kSequenceTrackProperty_SoloStatus,
                                    &status, sizeof(Boolean)),
        fail);
    
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackSetProperty()");
}

static VALUE
track_get_length (VALUE self)
{
    MusicTrack *track;
    MusicTimeStamp length;
    UInt32 sz;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    
    require_noerr(
        err = MusicTrackGetProperty(*track, kSequenceTrackProperty_TrackLength,
                                    &length, &sz),
        fail);
    
    if (sz == sizeof(MusicTimeStamp))
        return rb_float_new(length);
    else
        return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

static VALUE
track_set_length (VALUE self, VALUE rb_length)
{
    if (!PRIM_NUM_P(rb_length))
        rb_raise(rb_eTypeError, "Expected length to be a number.");
    MusicTrack *track;
    MusicTimeStamp length = NUM2DBL(rb_length);
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    
    require_noerr(
        err = MusicTrackSetProperty(*track, kSequenceTrackProperty_TrackLength,
                                    &length, sizeof(MusicTimeStamp)),
        fail);
    
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

static VALUE
track_get_resolution (VALUE self)
{
    MusicTrack *track;
    SInt16 res;
    UInt32 sz;
    OSStatus err;
    Data_Get_Struct(self, MusicTrack, track);
    
    require_noerr(
        err = MusicTrackGetProperty(*track, kSequenceTrackProperty_TimeResolution, &res, &sz),
        fail);
    
    if (sz == sizeof(SInt16))
        return INT2FIX(res);
    else
        return Qnil;

    fail:
    if (paramErr == err)
        rb_raise(rb_eArgError, "Resolution is only available to the tempo track.");
    else
        RAISE_OSSTATUS(err, "MusicTrackGetProperty()");
}

/* TrackCollection defns */

static MusicSequence*
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
    RAISE_OSSTATUS(err, "MusicSequenceGetTrackCount()");
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
    if (err == kAudioToolboxErr_TrackIndexError) {
      return Qnil;
    } else {
      RAISE_OSSTATUS(err, "MusicSequenceGetIndTrack()");
    }
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
    RAISE_OSSTATUS(err, "MusicSequenceGetTrackIndex()");
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
    RAISE_OSSTATUS(err, "MusicSequenceGetTempoTrack()");
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
    RAISE_OSSTATUS(err, "MusicSequenceDisposeTrack()");
}

/* MIDINoteMessage */

static void
midi_note_message_free (MIDINoteMessage *msg)
{
    if (msg) free(msg);
}

static VALUE
midi_note_message_alloc (VALUE class)
{
    MIDINoteMessage *msg;
    return Data_Make_Struct(class, MIDINoteMessage, 0, midi_note_message_free, msg);
}

static VALUE
midi_note_message_init (VALUE self, VALUE rb_opts)
{
    Check_Type(rb_opts, T_HASH);
    MIDINoteMessage *msg;
    VALUE rb_chn, rb_note, rb_vel, rb_rel_vel, rb_dur;

    Data_Get_Struct(self, MIDINoteMessage, msg);

    rb_chn = rb_hash_aref(rb_opts, rb_sChannel);
    msg->channel = FIXNUM_P(rb_chn) ? FIX2UINT(rb_chn) : 1;
    
    rb_note = rb_hash_aref(rb_opts, rb_sNote);
    if (FIXNUM_P(rb_note))
        msg->note = FIX2UINT(rb_note);
    else
        rb_raise(rb_eArgError, ":note is required.");
    
    rb_vel = rb_hash_aref(rb_opts, rb_sVelocity);
    msg->velocity = FIXNUM_P(rb_vel) ? FIX2UINT(rb_vel) : 64;
    
    rb_rel_vel = rb_hash_aref(rb_opts, rb_sReleaseVelocity);
    msg->releaseVelocity = FIXNUM_P(rb_rel_vel) ? FIX2UINT(rb_rel_vel) : 0;
    
    rb_dur = rb_hash_aref(rb_opts, rb_sDuration);
    msg->duration = (MusicTimeStamp) (PRIM_NUM_P(rb_dur)) ? NUM2DBL(rb_dur) : 1.0;
    
    return self;
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

static VALUE
midi_note_message_from_const (MIDINoteMessage *msg)
{
    VALUE rb_opts;
    rb_opts = rb_hash_new();
    rb_hash_aset(rb_opts, rb_sChannel, INT2NUM(msg->channel));
    rb_hash_aset(rb_opts, rb_sNote, INT2NUM(msg->note));
    rb_hash_aset(rb_opts, rb_sVelocity, INT2NUM(msg->velocity));
    rb_hash_aset(rb_opts, rb_sReleaseVelocity, INT2NUM(msg->releaseVelocity));
    rb_hash_aset(rb_opts, rb_sDuration, rb_float_new(msg->duration));
    return rb_funcall(rb_cMIDINoteMessage, rb_intern("new"), 1, rb_opts);
}

/* MIDIChannelMessage */

static void
midi_channel_message_free (MIDIChannelMessage *msg)
{
    if (msg) free(msg);
}

static VALUE
midi_channel_message_alloc (VALUE class)
{
    MIDIChannelMessage *msg;
    return Data_Make_Struct(class, MIDIChannelMessage, 0, midi_channel_message_free, msg);
}

static VALUE
midi_channel_message_init (VALUE self, VALUE rb_opts)
{
    Check_Type(rb_opts, T_HASH);
    MIDIChannelMessage *msg;
    VALUE rb_status, rb_data1, rb_data2;
    
    Data_Get_Struct(self, MIDIChannelMessage, msg);
    
    rb_status = rb_hash_aref(rb_opts, rb_sStatus);
    if (!FIXNUM_P(rb_status))
        rb_raise(rb_eArgError, ":status is required.");
    else
        msg->status = NUM2DBL(rb_status);
    
    rb_data1 = rb_hash_aref(rb_opts, rb_sData1);
    if (!NIL_P(rb_data1)) msg->data1 = (UInt8) FIX2INT(rb_data1);
    
    rb_data2 = rb_hash_aref(rb_opts, rb_sData2);
    if (!NIL_P(rb_data2)) msg->data2 = (UInt8) FIX2INT(rb_data2);
    
    return self;
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

static VALUE
midi_channel_message_from_const (MIDIChannelMessage *msg)
{
    VALUE rb_opts = rb_hash_new();
    switch(msg->status >> 4) {
    case 0xA: // key pressure
        rb_hash_aset(rb_opts, rb_sChannel, INT2NUM(msg->status ^ 0xA0));
        rb_hash_aset(rb_opts, rb_sNote, INT2NUM(msg->data1));
        rb_hash_aset(rb_opts, rb_sPressure, INT2NUM(msg->data2));
        return rb_funcall(rb_cMIDIKeyPressureMessage, rb_intern("new"), 1, rb_opts);
    case 0xB: // control change
        rb_hash_aset(rb_opts, rb_sChannel, INT2NUM(msg->status ^ 0xB0));
        rb_hash_aset(rb_opts, rb_sNumber, INT2NUM(msg->data1));
        rb_hash_aset(rb_opts, rb_sValue, INT2NUM(msg->data2));
        return rb_funcall(rb_cMIDIControlChangeMessage, rb_intern("new"), 1, rb_opts);
    case 0xC: // program change
        rb_hash_aset(rb_opts, rb_sChannel, INT2NUM(msg->status ^ 0xC0));
        rb_hash_aset(rb_opts, rb_sProgram, INT2NUM(msg->data1));
        return rb_funcall(rb_cMIDIProgramChangeMessage, rb_intern("new"), 1, rb_opts);
    case 0xD: // channel pressure
        rb_hash_aset(rb_opts, rb_sChannel, INT2NUM(msg->status ^ 0xD0));
        rb_hash_aset(rb_opts, rb_sPressure, INT2NUM(msg->data1));
        return rb_funcall(rb_cMIDIChannelPressureMessage, rb_intern("new"), 1, rb_opts);
    case 0xE: // pitch bend
        rb_hash_aset(rb_opts, rb_sChannel, INT2NUM(msg->status ^ 0xE0));
        rb_hash_aset(rb_opts, rb_sValue, INT2NUM(msg->data1));
        return rb_funcall(rb_cMIDIPitchBendMessage, rb_intern("new"), 1, rb_opts);
    default:
        rb_raise(rb_eRuntimeError, "Unrecognized message type.");
    }
}

/* ExtendedTempoEvent defns */
static VALUE
tempo_from_const (ExtendedTempoEvent *ev)
{
  VALUE rb_opts = rb_hash_new();
  rb_hash_aset(rb_opts, rb_sBpm, rb_float_new(ev->bpm));
  return rb_funcall(rb_cExtendedTempoEvent, rb_intern("new"), 1, rb_opts);
}

/* MusicEventIterator defns */
static void
iter_free (MusicEventIterator *iter)
{
    OSStatus err;
    require_noerr( err = DisposeMusicEventIterator(*iter), fail );
    return;
    
    fail:
    rb_warning("DisposeMusicEventIterator() failed with OSStatus %i.", (int) err);
}

static VALUE
iter_alloc (VALUE class)
{
    MusicEventIterator *iter;
    return Data_Make_Struct(rb_cMusicEventIterator, MusicEventIterator, 0, iter_free, iter);
}

static VALUE
iter_init (VALUE self, VALUE rb_track)
{
    MusicTrack *track;
    MusicEventIterator *iter;
    OSStatus err;
    Data_Get_Struct(rb_track, MusicTrack, track);
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = NewMusicEventIterator(*track, iter), fail );
    return self;
    
    fail:
    RAISE_OSSTATUS(err, "NewMusicEventIterator()");
}

static VALUE
iter_seek (VALUE self, VALUE rb_time)
{
    MusicEventIterator *iter;
    MusicTimeStamp ts;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    if (PRIM_NUM_P(rb_time))
        ts = NUM2DBL(rb_time);
    else
        rb_raise(rb_eArgError, "Expected first arg to be a number.");
    require_noerr( err = MusicEventIteratorSeek(*iter, ts), fail );
    return Qnil;

    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorSeek()");
}

static VALUE
iter_next (VALUE self)
{
    MusicEventIterator *iter;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorNextEvent(*iter), fail );
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorNextEvent()");
}

static VALUE
iter_prev (VALUE self)
{
    MusicEventIterator *iter;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorPreviousEvent(*iter), fail );
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorPreviousEvent()");
}

static VALUE
iter_has_current (VALUE self)
{
    MusicEventIterator *iter;
    Boolean has_cur;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorHasCurrentEvent(*iter, &has_cur), fail );
    return has_cur ? Qtrue : Qfalse;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorHasCurrentEvent()");
}

static VALUE
iter_has_prev (VALUE self)
{
    MusicEventIterator *iter;
    Boolean has_prev;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorHasPreviousEvent(*iter, &has_prev), fail );
    return has_prev ? Qtrue : Qfalse;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorHasPreviousEvent()");
}

static VALUE
iter_has_next (VALUE self)
{
    MusicEventIterator *iter;
    Boolean has_next;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorHasNextEvent(*iter, &has_next), fail );
    return has_next ? Qtrue : Qfalse;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorHasNextEvent()");
}

static VALUE
iter_get_time (VALUE self)
{
    MusicEventIterator *iter;
    MusicTimeStamp ts;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorGetEventInfo(*iter, &ts, NULL, NULL, NULL), fail );
    return rb_float_new(ts);
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorGetEventInfo()");
}

static VALUE
iter_set_time (VALUE self, VALUE rb_time)
{
    MusicEventIterator *iter;
    MusicTimeStamp ts = NUM2DBL(rb_time);
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorSetEventTime(*iter, ts), fail );
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorSetEventTime()");
}

static VALUE
iter_get_event (VALUE self)
{
    MusicEventIterator *iter;
    MusicEventType type;
    const void *data;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorGetEventInfo(*iter, NULL, &type, &data, NULL), fail );
    
    switch(type) {
    case kMusicEventType_NULL:
        return Qnil;
    case kMusicEventType_MIDINoteMessage:
        return midi_note_message_from_const((MIDINoteMessage*) data);
    case kMusicEventType_MIDIChannelMessage:
        return midi_channel_message_from_const((MIDIChannelMessage*) data);
    case kMusicEventType_ExtendedTempo:
        return tempo_from_const((ExtendedTempoEvent*) data);
    default:
        rb_raise(rb_eNotImpError, "Unsupported event type.");
        break;
    }
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorGetEventInfo()");
}

static VALUE
iter_set_event (VALUE self, VALUE rb_msg)
{
    MusicEventIterator *iter;
    MusicEventType type;
    const void *data;
    OSStatus err;
    
    Data_Get_Struct(self, MusicEventIterator, iter);
    
    if (THRQL(rb_cMIDINoteMessage, rb_msg)) {
        type = kMusicEventType_MIDINoteMessage;
        Data_Get_Struct(rb_msg, MIDINoteMessage, data);
    } else if (THRQL(rb_cMIDIChannelMessage, rb_msg)) {
        type = kMusicEventType_MIDIChannelMessage;
        Data_Get_Struct(rb_msg, MIDIChannelMessage, data);
    } else if (THRQL(rb_cExtendedTempoEvent, rb_msg)) {
        type = kMusicEventType_ExtendedTempo;
        ExtendedTempoEvent tmp;
        tmp.bpm = NUM2DBL(rb_funcall(rb_msg, rb_intern("bpm"), 0));
        data = &tmp;
    } else {
        rb_raise(rb_eTypeError, "Unrecognized event type");
    }
    
    require_noerr( err = MusicEventIteratorSetEventInfo(*iter, type, data), fail );
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorSetEventInfo()");
}

static VALUE
iter_delete_event (VALUE self)
{
    MusicEventIterator *iter;
    OSStatus err;
    Data_Get_Struct(self, MusicEventIterator, iter);
    require_noerr( err = MusicEventIteratorDeleteEvent(*iter), fail );
    return Qnil;
    
    fail:
    RAISE_OSSTATUS(err, "MusicEventIteratorDeleteEvent()");
}

/* Initialize extension */

void
Init_music_player ()
{
    /*
     * CoreMIDI
     */
    rb_mCoreMIDI = rb_define_module("CoreMIDI");
    rb_define_module_function(rb_mCoreMIDI, "get_number_of_destinations", core_midi_get_number_of_destinations, 0);
    rb_define_module_function(rb_mCoreMIDI, "get_destination", core_midi_get_destination, 1);
    
    /*
     * AudioToolbox
     */
    rb_mAudioToolbox = rb_define_module("AudioToolbox");
    
    /*
     * AudioToolbox exceptions
     */
    rb_eTrackNotFound = rb_define_class_under(rb_mAudioToolbox, "TrackNotFound", rb_eStandardError);
    rb_eEndOfTrack = rb_define_class_under(rb_mAudioToolbox, "EndOfTrack", rb_eStandardError);
    rb_eStartOfTrack = rb_define_class_under(rb_mAudioToolbox, "StartOfTrack", rb_eStandardError);
    rb_eNoSequence = rb_define_class_under(rb_mAudioToolbox, "NoSequence", rb_eStandardError);
    rb_eIllegalTrackDestination = rb_define_class_under(rb_mAudioToolbox, "IllegalTrackDestination", rb_eStandardError);
    
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
    rb_define_singleton_method(rb_cMusicTrack, "new", track_new, -1);
    rb_define_method(rb_cMusicTrack, "initialize", track_init, -1);
    rb_define_method(rb_cMusicTrack, "add_midi_note_message", track_add_midi_note_message, 2);
    rb_define_method(rb_cMusicTrack, "add_midi_channel_message", track_add_midi_channel_message, 2);
    rb_define_method(rb_cMusicTrack, "add_extended_tempo_event", track_add_extended_tempo_event, 2);
    rb_define_method(rb_cMusicTrack, "loop_info", track_get_loop_info, 0);
    rb_define_method(rb_cMusicTrack, "loop_info=", track_set_loop_info, 1);
    rb_define_method(rb_cMusicTrack, "offset", track_get_offset, 0);
    rb_define_method(rb_cMusicTrack, "offset=", track_set_offset, 1);
    rb_define_method(rb_cMusicTrack, "mute", track_get_mute, 0);
    rb_define_method(rb_cMusicTrack, "mute=", track_set_mute, 1);
    rb_define_method(rb_cMusicTrack, "solo", track_get_solo, 0);
    rb_define_method(rb_cMusicTrack, "solo=", track_set_solo, 1);
    rb_define_method(rb_cMusicTrack, "length", track_get_length, 0);
    rb_define_method(rb_cMusicTrack, "length=", track_set_length, 1);
    rb_define_method(rb_cMusicTrack, "resolution", track_get_resolution, 0);
    
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
    rb_cMIDIKeyPressureMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIKeyPressureMessage", rb_cMIDIChannelMessage);
    rb_cMIDIControlChangeMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIControlChangeMessage", rb_cMIDIChannelMessage);
    rb_cMIDIProgramChangeMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIProgramChangeMessage", rb_cMIDIChannelMessage);
    rb_cMIDIChannelPressureMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIChannelPressureMessage", rb_cMIDIChannelMessage);
    rb_cMIDIPitchBendMessage = rb_define_class_under(rb_mAudioToolbox, "MIDIPitchBendMessage", rb_cMIDIChannelMessage);
    rb_define_alloc_func(rb_cMIDIChannelMessage, midi_channel_message_alloc);
    rb_define_method(rb_cMIDIChannelMessage, "initialize", midi_channel_message_init, 1);
    rb_define_method(rb_cMIDIChannelMessage, "status", midi_channel_message_status, 0);
    rb_define_method(rb_cMIDIChannelMessage, "data1", midi_channel_message_data1, 0);
    rb_define_method(rb_cMIDIChannelMessage, "data2", midi_channel_message_data2, 0);
    
    /* AudioToolbox::ExtendedTempoEvent */
    rb_cExtendedTempoEvent = rb_define_class_under(rb_mAudioToolbox, "ExtendedTempoEvent", rb_cObject);
    
    /* AudioToolbox::MusicEventIterator */
    rb_cMusicEventIterator = rb_define_class_under(rb_mAudioToolbox, "MusicEventIterator", rb_cObject);
    rb_define_alloc_func(rb_cMusicEventIterator, iter_alloc);
    rb_define_method(rb_cMusicEventIterator, "initialize", iter_init, 1);
    rb_define_method(rb_cMusicEventIterator, "seek", iter_seek, 1);
    rb_define_method(rb_cMusicEventIterator, "next", iter_next, 0);
    rb_define_method(rb_cMusicEventIterator, "prev", iter_prev, 0);
    rb_define_method(rb_cMusicEventIterator, "current?", iter_has_current, 0);
    rb_define_method(rb_cMusicEventIterator, "next?", iter_has_next, 0);
    rb_define_method(rb_cMusicEventIterator, "prev?", iter_has_prev, 0);
    rb_define_method(rb_cMusicEventIterator, "time", iter_get_time, 0);
    rb_define_method(rb_cMusicEventIterator, "time=", iter_set_time, 1);
    rb_define_method(rb_cMusicEventIterator, "event", iter_get_event, 0);
    rb_define_method(rb_cMusicEventIterator, "event=", iter_set_event, 1);
    rb_define_method(rb_cMusicEventIterator, "delete", iter_delete_event, 0);
    
    /* Symbols */
    rb_sBeat = CSTR2SYM("beat");
    rb_sBpm = CSTR2SYM("bpm");
    rb_sChannel = CSTR2SYM("channel");
    rb_sData1 = CSTR2SYM("data1");
    rb_sData2 = CSTR2SYM("data2");
    rb_sDuration = CSTR2SYM("duration");
    rb_sNote = CSTR2SYM("note");
    rb_sLength = CSTR2SYM("length");
    rb_sLoopInfo = CSTR2SYM("loop_info");
    rb_sMute = CSTR2SYM("mute");
    rb_sNumber = CSTR2SYM("number");
    rb_sPressure = CSTR2SYM("pressure");
    rb_sProgram = CSTR2SYM("program");
    rb_sReleaseVelocity = CSTR2SYM("release_velocity");
    rb_sSamp = CSTR2SYM("samp");
    rb_sSecs = CSTR2SYM("secs");
    rb_sSolo = CSTR2SYM("solo");
    rb_sStatus = CSTR2SYM("status");
    rb_sValue = CSTR2SYM("value");
    rb_sVelocity = CSTR2SYM("velocity");
}
