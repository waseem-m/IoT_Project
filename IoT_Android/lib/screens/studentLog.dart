// @dart=2.9
import 'dart:async';

import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_midi/flutter_midi.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:flutterfire_samples/screens/feedback.dart';

int postsLen=0;
List<Timer> timerList=[];

class studentLog extends StatefulWidget
{
  static const routeName = '/saved';
  const studentLog({Key key,  User user})
      : _user = user,
        super(key: key);
  final User _user;

  @override
  _studentLog createState() => _studentLog();
}


class _studentLog extends State<studentLog> {
  bool _showLabels = true;
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

    super.dispose();
  }


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


  String dropdownvalue="Pick a tone";

  @override
  Widget build(BuildContext context) {
    CollectionReference users = FirebaseFirestore.instance.collection('Lessons');
    return Scaffold(
      backgroundColor: CustomColors.firebaseGrey,
      appBar: AppBar(
          backgroundColor: CustomColors.firebaseNavy,
          title: Text('Log of your pending lessons'),
          actions: <Widget>[




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
      drawer: Drawer(

          child: SafeArea(

            child: ListView(
                children: <Widget>[
                  Container(height: 20.0),

                  ListTile(title: Text("Current System:",style: TextStyle(fontSize: 16,
                    color: CustomColors.firebaseGrey,
                  )),
                    trailing: StreamBuilder<QuerySnapshot>(
                        stream: FirebaseFirestore.instance.collection('System').snapshots(),
                        builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                          if (snapshot.hasError) {
                            return Text('Something went wrong');
                          }

                          final posts = snapshot.data.docs.toList();


                          _showLabels = posts[0]["Sensors"];


                          return _showLabels ? Text(
                              ' SENSORS',
                              style: TextStyle(
                                color: Colors.black ,
                                fontSize: 20,

                              )) : Text(
                              'CAMERA',
                              style: TextStyle(
                                color: Colors.black ,
                                fontSize: 20,

                              ));
                        }),),
                  Divider(),
                  ListTile(title: Text("Switch to change system",style: TextStyle(fontSize: 16,
                      letterSpacing: 0.5,
                      color: CustomColors.firebaseYellow),),
                    trailing:   StreamBuilder<QuerySnapshot>(
                        stream: FirebaseFirestore.instance.collection('System').snapshots(),
                        builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                          if (snapshot.hasError) {
                            return Text('Something went wrong');
                          }

                          final posts = snapshot.data.docs.toList();


                          _showLabels = posts[0]["Sensors"];


                          return Switch(
                              value: _showLabels,
                              onChanged: (bool value) =>
                                  setState(()  { _showLabels= value;
                                  final FirebaseFirestore _firebase = FirebaseFirestore.instance;

                                  _firebase.collection('System').doc('0hYJMiVcjZuSJHH6fs8V'
                                  ).update({
                                    "Sensors": value,
                                  });
                                  }));
                        }),),
                  Divider(),
                  ListTile(title: Text("Points:",style: TextStyle(fontSize: 16,
                    color: CustomColors.firebaseGrey,
                  )),
                    trailing: StreamBuilder<QuerySnapshot>(
                        stream: FirebaseFirestore.instance.collection('Students').where("studentEmail", isEqualTo: widget._user.email).snapshots(),
                        builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                          if (snapshot.hasError) {
                            return Text('Something went wrong');
                          }

                          final posts = snapshot.data.docs.toList();
                          int x= posts.length;

                          int points = posts[0]["points"];


                          return Text(
                              ' $points Points' ,
                              style: TextStyle(
                                color: Colors.redAccent ,
                                fontSize: 15,

                              ));
                        }),)
                ]),
          )),
      body : SingleChildScrollView( child: new StreamBuilder<QuerySnapshot>(

        stream: users.where("studentEmail", isEqualTo: widget._user.email).orderBy('time',descending: true).snapshots(),
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
                DataColumn(label: Text(
                    'Lesson Notes',
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
                    'Your status',
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
                DataCell(  e.value['isConfirmed']? Text('Added',style: TextStyle(
                  color: Colors.black ,
                  fontSize: 15,
                )): IconButton(

                        icon: Icon(
                          Icons.add,
                          color: Colors.green,
                        ),
                        onPressed: () async =>  showDialog<String>(
                          context: context,
                          builder: (BuildContext dialogcontext) => AlertDialog(
                            title:  Text('Are you sure you would like to play this track? '),
                            content:  Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              mainAxisSize: MainAxisSize.min,
                              children: <Widget>[SizedBox(height: 15.0),
                                Text( convert(e.value['teacherKeys'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
                                  color: Colors.white ,
                                  fontSize: 14,
                                  letterSpacing: 0.5,
                                )),
                                SizedBox(height: 4.0),
                              ],
                            ),
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
                                    Navigator.pop(dialogcontext, 'YES');
                                    String docId=e.value.id;
                              final FirebaseFirestore _firebase = FirebaseFirestore.instance;
                              String str = widget._user.uid;
                              await _firebase.collection('Lessons').doc(docId).update({
                                "isConfirmed": true,
                                "time": FieldValue.serverTimestamp()
                              });
                              Navigator.pop(dialogcontext, 'YES');
                                }
                                ,
                                child: const Text('YES'),
                              ),

                            ],
                          ),
                        ),
                      )

                ),
                DataCell(Text(convert(e.value['teacherKeys'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
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
                      playSong(convert(e.value['teacherKeys'].toList()));
                    }
                )
                ),
                DataCell(!e.value['isBegin'] ? Text('Haven\'t played yet',style: TextStyle(

                  fontSize: 14,
                  color: Colors.black ,
                )):Text(convert(e.value['studentKeys'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(

                  fontSize: 14,
                  color: Colors.black ,
                ))
                ),
                DataCell(IconButton(
                    icon: Icon(
                      Icons.delete,
                      color: Colors.black ,),
                    onPressed: () async =>  showDialog<String>(
                      context: context,
                      builder: (BuildContext dialogcontext) => AlertDialog(
                        title:  Text('Are you sure you would like to delete the following lesson ?'),
                        content:  Text(convert(e.value['teacherKeys'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
                          color: Colors.white ,
                          fontSize: 14,
                        )),
                        actions: <Widget>[
                          TextButton(
                            onPressed: () async {
                              Navigator.pop(dialogcontext, 'CANCEL');
                            },
                            child: const Text('CANCEL'),
                          ),
                          TextButton(
                            onPressed: () async {
                              String docId=e.value.id;
                              final FirebaseFirestore _firebase = FirebaseFirestore
                                  .instance;
                              String str = widget._user.uid;
                              await _firebase.collection('Lessons').doc(docId).delete();
                              Navigator.pop(dialogcontext, 'YES');
                            },
                            child: const Text('YES'),
                          ),

                        ],
                      ),
                    )),

                )
              ] , color: MaterialStateProperty.resolveWith<Color>((Set<MaterialState> states) {


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