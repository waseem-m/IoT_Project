// @dart=2.9

//import 'package:audioplayers/audio_cache.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutterfire_samples/screens/feedback.dart';
import 'package:flutter/services.dart';
import 'package:flutter/material.dart';
import 'package:mutex/mutex.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';

List<int> song1 = [3,2,1,2,3,3,3,2,2,2,3,3,3,3,2,1,2,3,3,3,3,2,2,3,2,1];
List<int> song2 = [3,3,3,3,3,3,3,5,1,2,3,4,4,4,4,4,3,3,3];
List<int> song3 = [7];
List<int> song4 = [5,3];
List<int> song5 = [7,1];
List<int> song6 = [1,1,1];
List<int> song7 = [2,2,2];
List<int> song8 = [1,3,3];
List<int> song9 = [1,4,5];
List<int> song10 = [1,7,1];
int c = 0;




class songs extends StatefulWidget {

  static const routeName = '/songs';
  const songs({Key key,  User user})
      : _user = user,
        super(key: key);

  final User _user;
 // static final AudioCache player = new AudioCache();
  @override

  static void play(int c) async {
 //await  player.play('note$c.wav');
  }

  _SongsState createState() => _SongsState();
}



class _SongsState extends State<songs>
{  User _user;
  final m = Mutex();
  @override
  void initState() {
    //songs.player.loadAll(
      //  ['note1.wav', 'note2.wav', 'note3.wav', 'note4.wav', 'note5.wav' , 'note6.wav', 'note7.wav']);
    _user = widget._user;
    super.initState();
  }

  @override
  void dispose() {
    super.dispose();
    //songs.player.clearCache();
  }

  void simonPlay (int index, int c) async {

      await Future.delayed(Duration(milliseconds: index *  500), () {
        songs.play(c);
      });
  }

  void playSequence(List<int> sequence) async {
    try {
      // critical section with asynchronous code
     for (var i = 0; i < sequence.length ; i++) {

          simonPlay(i,sequence[i]);
      }
  } finally {
    m.release();
    }
    }


  Widget myTone1(Color myColor1,Color myColor2, int myNumber)
  {
    return Expanded(
       child : AbsorbPointer ( absorbing: false,
        child: TextButton(
          style: ButtonStyle(backgroundColor: MaterialStateProperty.all(myColor1)),
          onPressed: () async {

                switch (myNumber) {
                  case 1:
                    await m.acquire();
                    playSequence(song1);
                    break;
                  case 2:
                    await m.acquire();
                    playSequence(song2);
                    break;

                  case 3:
                    await m.acquire();
                    playSequence(song3);
                    break;

                  case 4:
                    await m.acquire();
                    playSequence(song4);
                    break;

                  case 5:
                    await m.acquire();
                    playSequence(song5);
                    break;

                  case 6:
                    await m.acquire();
                    playSequence(song6);
                    break;

                  case 7:
                  // do something else
                    await m.acquire();
                    playSequence(song7);
                    break;
                  case 8:
                    await m.acquire();
                    playSequence(song8);
                    break;

                  case 9:
                    await m.acquire();
                    playSequence(song9);
                    break;

                  case 10:
                    await m.acquire();
                    playSequence(song10);

                };
                setState((){});

              },

            child:  Text(
              'tone$myNumber',
              style: TextStyle(
                color: myColor2 ,
                fontSize: 20,
                letterSpacing: 0.5,
              ),
            ),
        ),
       ),
    );
  }

//todo: install songs & put a mutex
  Widget myTone(int num) {
    return Expanded(
        child:
        Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: <Widget>[
             myTone1( CustomColors.firebaseNavy,CustomColors.firebaseOrange, num )
            ,
            myTone1( CustomColors.firebaseOrange,CustomColors.firebaseNavy, num + 1)
          ],
        ),);


  }


  Widget myToneOpposite(int num) {
    return Expanded(
      child:
      Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: <Widget>[
          myTone1(CustomColors.firebaseOrange,CustomColors.firebaseNavy, num)
          ,
          myTone1(CustomColors.firebaseNavy,CustomColors.firebaseOrange, num + 1)

        ],
      ),);


  }

  String _dropDownValue="Pick a student";
  String _value = "Pick a tone";
  List <String> tones = ["Tone 1", "Tone 2","Tone 3","Tone 4", "Tone 5","Tone 6","Tone 7","Tone 8","Tone 9","Tone 10"];

  @override
  Widget build(BuildContext context)
  {


    // Set landscape orientation
    SystemChrome.setPreferredOrientations([
      DeviceOrientation.landscapeLeft,
      DeviceOrientation.landscapeRight,
    ]);

    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text(''),

          backgroundColor: CustomColors.firebaseNavy,
          actions: <Widget>[
            StreamBuilder<QuerySnapshot>(
              stream: FirebaseFirestore.instance.collection('Students').snapshots(),
              builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                if (snapshot.hasError) {
                  return Text('Something went wrong');
                }
                List<String> uns=[];
                final posts = snapshot.data.docs.asMap();
                for(var v in posts.values) {
                  uns.add(v["studentEmail"]);
                }
                return DropdownButton<String>
                  (
                  hint: _dropDownValue == null
                      ? Text('Dropdown')
                      : Text(
                    _dropDownValue,
                    style: TextStyle(fontSize: 16,
                        letterSpacing: 0.5,
                        color: CustomColors.firebaseOrange),
                  ),

                  style: TextStyle(color: CustomColors.firebaseOrange),
                  items: uns.map(
                        (val) {
                      return DropdownMenuItem<String>(
                        value: val,
                        child: Text(val),
                      );
                    },
                  ).toList(),
                  onChanged: (val) {
                    setState(
                          () {
                        _dropDownValue = val;
                      },
                    );
                  },
                );
              },
            ),

    IconButton(
    icon: Icon(
    Icons.add_circle_outlined,
    color: CustomColors.firebaseOrange,
    ),
    onPressed: () async {
    // do something
    },
    )
,
      DropdownButton<String>
        (
        hint: _value == null
            ? Text('No students assigned')
            : Text(
          _value,
          style: TextStyle(fontSize: 16,
              letterSpacing: 0.5,
              color: CustomColors.firebaseOrange),
        ),

        style: TextStyle(color: CustomColors.firebaseOrange),
        items: tones.map(
              (val) {
            return DropdownMenuItem<String>(
              value: val,
              child: Text(val),
            );
          },
        ).toList(),
        onChanged: (val) {
          setState(
                () {
              _value = val;
            },
          );
        },
      ),
            IconButton(

              icon: Icon(
                Icons.send,
                color: CustomColors.firebaseOrange,
              ),
              onPressed: () async {
                // do something
                if( _dropDownValue != "Pick a student" && _value!="Pick a tone") {
                  List<int> pressedToneList = [];
                  final FirebaseFirestore _firebase = FirebaseFirestore
                      .instance;
                  switch (_value) {
                    case "Tone 1":
                      pressedToneList = song1;
                      break;

                    case "Tone 2":
                      pressedToneList = song2;
                      break;

                    case "Tone 3":
                      pressedToneList = song3;
                      break;

                    case "Tone 4":
                      pressedToneList = song4;
                      break;

                    case "Tone 5":
                      pressedToneList = song5;
                      break;

                    case "Tone 6":
                      pressedToneList = song6;
                      break;

                    case "Tone 7":
                      pressedToneList = song7;
                      break;
                    case "Tone 8":
                      pressedToneList = song8;
                      break;

                    case "Tone 9":
                      pressedToneList = song9;
                      break;

                    case "Tone 10":
                      pressedToneList = song10;
                  }
                  //String studentUid="";
                  // FirebaseFirestore.instance
                  //     .collection('Students').where("username", isEqualTo: _dropDownValue)
                  //     .get()
                  //     .then((QuerySnapshot querySnapshot) {
                  //   studentUid=querySnapshot.docs[0]["uid"];
                  // });

                  String str = widget._user.uid;
                  c++;
                  String currentDoc = "";
                  List<int> student = [];
                  await _firebase.collection('Lessons').add(
                      {
                        "teacherKeys": pressedToneList,
                        "teacherUid": str,
                        "studentKeys": student,
                        "studentEmail": _dropDownValue,
                        "isBegin": false,
                        "time": FieldValue.serverTimestamp()
                      }).then((value) {
                    currentDoc = value.id;
                  });
                  pressedToneList.clear();
                  Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) =>
                        feedbackScreen(user: _user, docId: currentDoc,)),
                  );
                }
              },
            ),
            IconButton(
              icon: Icon(
                Icons.add,
                color: CustomColors.firebaseNavy,
              ),
              onPressed: () async {
                // do something
              },
            ),
            IconButton(
              icon: Icon(
                Icons.home,
                color:Colors.white ,
              ),
              onPressed: () async {
                // do something
                Navigator.pop(context);
              },
            ),
          ],
        ),
        body: SafeArea(
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: <Widget>[
              myTone(1),
              myToneOpposite(3),
              myTone(5),
                myToneOpposite(7),
                myTone(9),

            ],
          ),
        ),
      ),
    );
  }
}




