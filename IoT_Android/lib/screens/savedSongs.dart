// @dart=2.9
import 'dart:async';

import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_midi/flutter_midi.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:mutex/mutex.dart';
import 'package:flutterfire_samples/screens/feedback.dart';
import 'package:rich_text_controller/rich_text_controller.dart';

int postsLen=0;
List<Timer> timerList=[];



class savedScreen extends StatefulWidget
{
  static const routeName = '/saved';
  const savedScreen({Key key,  User user})
      : _user = user,
        super(key: key);
  final User _user;

  @override
  _savedState createState() => _savedState();
}
class Tone {
  Tone( this.name,  this.tone);
  final String name;
  final List<dynamic> tone;
}

class _savedState extends State<savedScreen> {

  User _user;
  @override
  initState()  {
    _user = widget._user;
    FlutterMidi().unmute();
    rootBundle.load("assets/sounds/Piano.sf2").then((sf2) {
      FlutterMidi().prepare(sf2: sf2, name: "Piano.sf2");
    });

    super.initState();
  }
  void dispose() {
    // Clean up the controller when the widget is disposed.
    //dropdownvalue="Pick a tone";
    super.dispose();
  }

  Map<RegExp, TextStyle> patternUser = {
    RegExp(r"\B@[,]+\b"):
    TextStyle(color: Colors.amber, fontWeight: FontWeight.bold)
  };

  void playNote (int index, int c) {
    print("playing index=$index");
    Timer t = Timer(Duration(milliseconds: index * 500), () async {
      await FlutterMidi().playMidiNote(midi: c);
     await print("Finished playing index=$index");
    });
    timerList.add(t);
    print("Added index=$index");

  }

  void playSong (List<String> thelist)
  {
    for(int i=0; i< timerList.length; i++){
      timerList[i].cancel();
    }
    timerList.clear();

    for(int i=0;i<thelist.length;i++)
    {
      int note=0;
      switch (thelist[i]) {
        case "C6":
          note=60 +24;
          break;

        case "C♯6":
          note=85;
          break;

        case "D6":
          note=86;
          break;

        case "D♯6":
          note=87;
          break;

        case "E6":
          note=88;
          break;

        case "F6":
          note=89;
          break;

        case "F♯6": //a b c d e f g h i j
          note=90;
          break;
        case "G6":
          note=91;
          break;

        case "G♯6":
          note=92;
          break;

        case "A6":
          note=93;
          break;
        case "A♯6":
          note=94;
          break;
        case "B6":
          note=95;
          break;
        case "C7":
          note=96;
          break;
      }
      playNote(i,note);
    }
  }
  List<String> convert (List<dynamic> thelist)
  { List<String> newList=[];


    for (int i = 0; i < thelist.length; i++) {
      switch (thelist[i]) {
        case "a":
          newList.add("C6");
          break;

        case "b":
          newList.add("C♯6");
          break;

        case "c":
          newList.add("D6");
          break;

        case "d":
          newList.add("D♯6");
          break;

        case "e":
          newList.add("E6");
          break;

        case "f":
          newList.add("F6");
          break;

        case "g":
          newList.add("F♯6");
          break;
        case "h":
          newList.add("G6");
          break;

        case "i":
          newList.add("G♯6");
          break;

        case "j":
          newList.add("A6");
          break;
        case "k":
          newList.add("A♯6");
          break;
        case "l":
          newList.add("B6");
          break;
        case "m":
          newList.add("C7");
          break;
      }
    }

  return newList;
  }
  String _dropDownValue="Pick a student";
  String dropdownvalue="Pick a tone";

  @override
  Widget build(BuildContext context) {
    CollectionReference users = FirebaseFirestore.instance.collection('Tones');
    // collection('Users').document('uidHere').collection('destination').snapshots()

    return Scaffold(
      backgroundColor: CustomColors.firebaseGrey,
      appBar: AppBar(
          backgroundColor: CustomColors.firebaseNavy,
          title: Text('Log of your tones collection'),
          actions: <Widget>[



            StreamBuilder<QuerySnapshot>(
              stream: FirebaseFirestore.instance.collection('Tones').where("teacherUid", isEqualTo: widget._user.uid).orderBy('time',descending: true).snapshots(),
              builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                if (snapshot.hasError) {
                  return Text('Something went wrong');
                }


                if (snapshot.connectionState == ConnectionState.waiting) {
                  return Text('Loading');
                }

                final posts = snapshot.data.docs.toList();
                List<String> names = [];
                List<String> tones = [];
                int c = 0;
                for (int i = 0; i < posts.length; i++) {
                  tones.add('$i');
                }
                for (int i = 0; i < posts.length; i++) {
                  names.add((i+1).toString()+ ": " + posts[i]["name"] );
                }
                return DropdownButton<String>
                  (
                  hint: Text(
                    dropdownvalue,
                    style: TextStyle(fontSize: 16,
                        letterSpacing: 0.5,
                        color: Colors.white),
                  ),

                  style: TextStyle(color: Colors.white),
                  items: tones.map(
                        (val) {
                      return DropdownMenuItem<String>(
                        value: (int.parse(val) +1).toString(),
                        child: Text(names[int.parse(val)]),
                      );
                    },
                  ).toList(),
                  onChanged: (val) {
                    setState(
                          () {
                        dropdownvalue = val;
                      },
                    );
                  },
                );
              }),
    StreamBuilder<QuerySnapshot>(
    stream: FirebaseFirestore.instance.collection('Tones').where("teacherUid", isEqualTo: widget._user.uid).orderBy('time',descending: true).snapshots(),
    builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
    if (snapshot.hasError) {
    return Text('Something went wrong');
    }


    if (snapshot.connectionState == ConnectionState.waiting) {
      return Center(
        child: CircularProgressIndicator(),
      );
    }

    final posts = snapshot.data.docs.toList();

    List<List<dynamic>> tones = [];
    for (int i = 0; i < posts.length; i++) {
    tones.add(posts[i]["tone"] );
    }

    return IconButton(

    icon: Icon(
    Icons.send,
    color: Colors.white,
    ),
        onPressed: () async =>  await showDialog<void>(
          context: context,
          builder: (BuildContext dialcontext) {
            int selectedRadio = 0;
            return AlertDialog(
              content: StatefulBuilder(
                builder: (BuildContext dialcontext, StateSetter setState) {
                  return Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    mainAxisSize: MainAxisSize.min,
                    children: <Widget>[
                      Text(dropdownvalue!="Pick a tone" ? 'Are you sure you would like to send this track? ': 'Please pick a tone!'),
                      SizedBox(height: 15.0),
                      Text( dropdownvalue!="Pick a tone" ? convert(tones[int.parse(dropdownvalue) - 1].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''): '',style: TextStyle(
                        color: Colors.white ,
                        fontSize: 14,
                        letterSpacing: 0.5,
                      )),
                      SizedBox(height: 20.0),
                      Text( 'Please Pick A Student',style: TextStyle(
                        color: Colors.grey ,
                        fontSize: 14,
                        letterSpacing: 0.5,
                      )),
                      StreamBuilder<QuerySnapshot>(
                        stream: FirebaseFirestore.instance.collection('Students').snapshots(),
                        builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                          if (snapshot.hasError) {
                            return Text('Something went wrong');
                          }

                          if (snapshot.connectionState == ConnectionState.waiting) {
                            return Center(
                              child: CircularProgressIndicator(),
                            );
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
                                  color: Colors.white),
                            )
                            ,
                            style: TextStyle(color: Colors.white),
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
                      SizedBox(height: 4.0),



                    ],

                  );



                },


              ),
              actions: <Widget>[
                TextButton(
                  onPressed: () async {
                    Navigator.pop(dialcontext, 'CANCEL');
                  }
                  ,
                  child: const Text('CANCEL'),
                ),
                TextButton(
                onPressed: () async {

              if(  dropdownvalue!="Pick a tone" && _dropDownValue!="Pick a student") {

                final FirebaseFirestore _firebase = FirebaseFirestore
                    .instance;
                String str = widget._user.uid;
                String currentDoc = "";
                List<int> student = [];
                await _firebase.collection('Lessons').add(
                    {
                      "teacherKeys": tones[int.parse(dropdownvalue) - 1] ,
                      "teacherUid": str,
                      "studentKeys": student,
                      "studentEmail": _dropDownValue,
                      "isBegin": false,
                      "isConfirmed": false,
                      "time": FieldValue.serverTimestamp()
                    }).then((value) {
                  currentDoc = value.id;
                });

                Navigator.pop(dialcontext, 'YES');


                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) =>
                      feedbackScreen(user: _user, docId: currentDoc,)),
                );
              }

            }
            ,
            child: const Text('YES'),),],
            );
          },
        ),

    );
    }),


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
          ]
      ),
      body : SingleChildScrollView( child: new StreamBuilder<QuerySnapshot>(
        //stream: users.where("teacherUid", isEqualTo: widget._user.uid).snapshots(),

        stream: users.where("teacherUid", isEqualTo: widget._user.uid).orderBy('time',descending: true).snapshots(),
        builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
          if (snapshot.hasError) {
            return Text('Something went wrong');
          }

          if (snapshot.connectionState == ConnectionState.waiting) {
            return Center(
              child: CircularProgressIndicator(),
            );
          }
          final posts = snapshot.data.docs.asMap();
           postsLen= posts.length;

          return Container(
              padding: EdgeInsets.all(10),
          child: SingleChildScrollView(
          scrollDirection: Axis.horizontal,
            child: DataTable(

              columns: const <DataColumn>[
                DataColumn(label: Text(' ')),
                DataColumn(label: Text('Track Name',
                    style: TextStyle(
                      color: Colors.black ,
                      fontSize: 16,
                      letterSpacing: 0.5,
                    ))),

                DataColumn(label: Text(
                    'Track Notes',
                    style: TextStyle(
                      color: Colors.black ,
                      fontSize: 16,
                      letterSpacing: 0.5,
                    ))),
                DataColumn(label: Text(
                    'Play',
                    style: TextStyle(
                      color: Colors.black ,
                      fontSize: 16,
                      letterSpacing: 0.5,
                    ))),
                DataColumn(label: Text(
                    '',
                    style: TextStyle(
                      color: Colors.black ,
                      fontSize: 16,
                      letterSpacing: 0.5,
                    ))),


              ],
              rows: posts.entries
                  .map((e) => DataRow(cells: [
                    DataCell(IconButton(
                        icon: Icon(
                          Icons.audiotrack,
                          color: Colors.orangeAccent ,)
                    )),
                    DataCell(

                 Text('#'+(e.key +1).toString()+'   ' +e.value['name'] ,style: TextStyle(
                      color: Colors.black ,
                      fontSize: 14,
                    )),


                ),
                DataCell(Text(convert(e.value['tone'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
                  color: Colors.black ,
                  fontSize: 14,

                ))),

                DataCell(IconButton(
                    icon: Icon(
                      Icons.volume_up,

                      color: Colors.redAccent ,),
                    onPressed: () {
                      FlutterMidi().unmute();


                        // critical section
                        playSong(convert(e.value['tone'].toList()));


                    }
                )),
                DataCell(IconButton(
                    icon: Icon(
                      Icons.delete,

                      color: Colors.black ,),
                    onPressed: () async =>  showDialog<String>(
                        context: context,
                        builder: (BuildContext dialogcontext) => AlertDialog(
                        title:  Text('Are you sure you would like to delete '+ e.value['name'].toString() +' ?'),
                        content:  Text(convert(e.value['tone'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
                          color: Colors.white ,
                          fontSize: 14,
                        )),
                        actions: <Widget>[
                          TextButton(
                            onPressed: () async {
                              Navigator.pop(dialogcontext, 'CANCEL');
                            }
                            ,

                            child: const Text('CANCEL'),
                          ),
                          TextButton(
                            onPressed: () async {
                              String docId=e.value.id;

                                final FirebaseFirestore _firebase = FirebaseFirestore
                                    .instance;
                                String str = widget._user.uid;


                                await _firebase.collection('Tones').doc(docId).delete();



                              Navigator.pop(dialogcontext, 'YES');
                            }
                            ,
                            child: const Text('YES'),
                          ),

                        ],
                      ),
          )),

                )
              ] , color: MaterialStateProperty.resolveWith<Color>((Set<MaterialState> states) {

/*
                else {
                  if(lengthDiff==0)
                   return Theme.of(context).colorScheme.surface.withGreen(110);

                  else {
                    if(studentKeys > 0)
                   return Theme.of(context).colorScheme.surface.withRed(255).withGreen(144).withOpacity(0.6);
                    else
                      return Theme.of(context).colorScheme.surface.withRed(130);
                  }
                }
                */

                //return null;  // Use the default value.
              }),))
                  .toList(),
            ),
          ),
          );


        },
      ),),

    );
  }
}


