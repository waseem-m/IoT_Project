// @dart=2.9

import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:rich_text_controller/rich_text_controller.dart';


class feedbackScreen extends StatefulWidget
{
  static const routeName = '/feedback';
  const feedbackScreen({Key key,  User user, String docId})
      : _user = user,
       _docId = docId,
        super(key: key);
  final User _user;
  final String _docId;
  @override
  _feedbackState createState() => _feedbackState();
}

class _feedbackState extends State<feedbackScreen> {

  RichTextController _controller;

  @override
  initState()  {
    _controller = RichTextController(
      patternMap: patternUser,

    );
    super.initState();
  }

  Map<RegExp, TextStyle> patternUser = {
    RegExp(r"\B@[,]+\b"):
    TextStyle(color: Colors.amber, fontWeight: FontWeight.bold)
  };
  @override
  Widget build(BuildContext context) {
    CollectionReference users = FirebaseFirestore.instance.collection('Lessons');
    // collection('Users').document('uidHere').collection('destination').snapshots()


    
    List<String> convert (List<dynamic> thelist)
    { List<String> newList=[];
      
      for(int i=0;i<thelist.length;i++) {
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
            //str = "d";
            newList.add("D♯6");
            break;

          case "e":
            //str = "e";
            newList.add("E6");
            break;

          case "f":
            //str = "f";
            newList.add("F6");
            break;

          case "g": //a b c d e f g h i j
            //str = "g";
            newList.add("F♯6");
            break;
          case "h":
            //str = "h";
            newList.add("G6");
            break;

          case "i":
            //str = "i";
            newList.add("G♯6");
            break;

          case "j":
            //str = "j";
            newList.add("A6");
            break;
          case "k":
            //str = "k";
            newList.add("A♯6");
            break;
          case "l":
            //str = "l";
            newList.add("B6");
            break;
          case "m":
            //str = "m";
            newList.add("C7");
            break;
        }
      }
      return newList;
    }

    return Scaffold(
        backgroundColor: CustomColors.firebaseGrey,
        appBar: AppBar(
            backgroundColor: CustomColors.firebaseNavy,
            title: Text('Log of your students'),
            actions: <Widget>[
              IconButton(
                icon: Icon(
                  Icons.circle,
                  color:Colors.grey ,
                ),
                onPressed: () async {
                  // do something

                },
              ),
              Text(
                'Waiting',
                style: TextStyle(
                  color: Colors.grey,
                  fontSize: 16,
                ),
              ),
              IconButton(
                icon: Icon(
                  Icons.circle,
                  color:Colors.green ,
                ),
                onPressed: () async {
                  // do something

                },
              ),
              Text(
                'Excellent',
                style: TextStyle(
                  color: Colors.green,
                  fontSize: 16,
                ),
              ),

              IconButton(
                icon: Icon(
                  Icons.circle,
                  color:Colors.orange ,
                ),
                onPressed: () async {
                  // do something

                },
              ),
              Text(
                'Average',
                style: TextStyle(
                  color: Colors.orange,
                  fontSize: 16,
                ),
              ),
              IconButton(
                icon: Icon(
                  Icons.circle,
                  color:Colors.red ,
                ),
                onPressed: () async {
                  // do something

                },
              ),
              Text(
                'Poor Performance ',
                style: TextStyle(
                  color: Colors.red,
                  fontSize: 19,
                ),
              ),


              IconButton(
                icon: Icon(
                  Icons.arrow_back_sharp,
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
          return Container(
              padding: EdgeInsets.all(10),
          child: SingleChildScrollView(
          scrollDirection: Axis.horizontal,
            child: DataTable(
              columns: const <DataColumn>[
                DataColumn(label: Text(' ')),
                DataColumn(label: Text(
                'My history',
                style: TextStyle(
                color: Colors.black ,
                fontSize: 16,
                letterSpacing: 0.5,
                ))),
                DataColumn(label: Text(
          'Student Email',
          style: TextStyle(
          color: Colors.black ,
          fontSize: 16,
          letterSpacing: 0.5,
          ))),
                DataColumn(label: Text(
                    'Student Keys',
                    style: TextStyle(
                      color: Colors.black ,
                      fontSize: 16,
                      letterSpacing: 0.5,
                    ))),
              ],
              rows: posts.entries
                  .map((e) => DataRow(cells: [DataCell(
                  IconButton(
                icon: Icon(
                  Icons.circle,
                color: 0 == e.value['studentKeys'].toList().length ? Colors.grey : e.value['teacherKeys'].toList().length == e.value['studentKeys'].toList().length
                && e.value['studentKeys'].toList()[e.value['studentKeys'].toList().length-1] == e.value['teacherKeys'].toList()[e.value['teacherKeys'].toList().length-1]
                    ? Colors.green : (e.value['studentKeys'].toList().length>0 && ( e.value['studentKeys'].toList()[0] == e.value['teacherKeys'].toList()[0]) ? Colors.orangeAccent : Colors.red),
              ))),
            DataCell(Text(convert(e.value['teacherKeys'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
              color: Colors.black ,
              fontSize: 14,

            ))),
                DataCell(Text(e.value['studentEmail'],
                    style: TextStyle(
                      color: Colors.black ,
                      fontSize: 14,

                    ))),
                DataCell( e.value['studentKeys'].toList().length!=0? Text(convert(e.value['studentKeys'].toList()).toString().replaceAll(',','  |  ').replaceAll('[','').replaceAll(']',''),style: TextStyle(
                  color: Colors.black ,
                  fontSize: 14,

                )):Text('No response yet', style: TextStyle(
                  color: Colors.black ,
                  fontSize: 14,

                )) ),


              ] , color: MaterialStateProperty.resolveWith<Color>((Set<MaterialState> states) {
                String doc_str = e.value.reference.id;

                if(doc_str ==widget._docId )
                  return Theme.of(context).colorScheme.surface.withGreen(110);
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