__author__ = 'Cosmo Harrigan'

from flask import abort, json, current_app
from flask.ext.restful import Resource, reqparse, marshal
from opencog.atomspace import Handle
from mappers import *
from flask.ext.restful.utils import cors

class AtomAPI(Resource):
    # This is because of https://github.com/twilio/flask-restful/issues/134
    @classmethod
    def new(cls, atomspace):
        cls.atomspace = atomspace
        return cls

    def __init__(self):
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('callback', type=str, location='args')
        super(AtomAPI, self).__init__()

    # Set CORS headers to allow cross-origin access (https://github.com/twilio/flask-restful/pull/131):
    @cors.crossdomain(origin='*')
    def get(self, id):
        """
        Returns the atom for the given handle
        Uri: atoms/[id]

        :param id: Atom handle

        :return atoms: Returns a JSON representation of an atom list containing the atom.

        Example:

        {
          'atoms':
          {
            'handle': 6,
            'name': '',
            'type': 'InheritanceLink',
            'outgoing': [2, 1],
            'incoming': [],
            'truthvalue':
              {
                'type': 'simple',
                'details':
                  {
                    'count': '0.4000000059604645',
                    'confidence': '0.0004997501382604241',
                    'strength': '0.5'
                  }
              },
          'attentionvalue':
            {
              'lti': 0,
              'sti': 0,
              'vlti': false
            }
          }
        }
        """

        try:
            atom = self.atomspace[Handle(id)]
        except IndexError:
            abort(404, 'Handle not found')

        json_data = {'atoms': marshal(atom, atom_fields)}
        
        # if callback function supplied, pad the JSON data (i.e. JSONP):
        args = self.reqparse.parse_args()
        callback = args.get('callback')
        if callback is not None:
            response = str(callback) + '(' + json.dumps(json_data) + ');'
            return current_app.response_class(response, mimetype='application/javascript')
        else:
            return current_app.response_class(json.dumps(json_data), mimetype='application/json')
        
    def put(self, id):
        """
        Updates the AttentionValue (STI, LTI, VLTI) or TruthValue of an atom
        Uri: atoms/[id]

        :param id: Atom handle
        Include data with the PUT request providing a JSON representation of the updated attributes.
        Valid data elements:

        truthvalue (optional) TruthValue, formatted as follows:
            type (required) TruthValue type (only 'simple' is currently available)
                            see http://wiki.opencog.org/w/TruthValue
            details (required) TruthValue parameters, formatted as follows:
                strength (required)
                count (required)
        attentionvalue (optional) AttentionValue, formatted as follows:
            sti (optional) Short-Term Importance
            lti (optional) Long-Term Importance
            vlti (optional) Very-Long Term Importance

        Example:

        {
          'truthvalue':
          {
            'type': 'simple',
            'details':
              {
                'strength': 0.005,
                'count': 0.8
              }
          },
        'attentionvalue':
          {
            'sti': 9,
            'lti': 2,
            'vlti': True
          }
        }

        :return atoms: Returns a JSON representation of an atom list containing the atom.
        Example:

        { 'atoms':
          {
            'handle': 6,
            'name': '',
            'type': 'InheritanceLink',
            'outgoing': [2, 1],
            'incoming': [],
            'truthvalue':
              {
                'type': 'simple',
                'details':
                  {
                    'count': '0.4000000059604645',
                    'confidence': '0.0004997501382604241',
                    'strength': '0.5'
                  }
              },
            'attentionvalue':
              {
                'lti': 0,
                'sti': 0,
                'vlti': false
              }
            }
          }
        }
        """

        if Handle(id) not in self.atomspace:
            abort(404, 'Handle not found')

        # Prepare the atom data
        data = reqparse.request.get_json()

        if 'truthvalue' not in data and 'attentionvalue' not in data:
            abort(400, 'Invalid request: you must include a truthvalue or attentionvalue parameter')

        if 'truthvalue' in data:
            tv = ParseTruthValue.parse(data)
            self.atomspace.set_tv(h=Handle(id), tv=tv)

        if 'attentionvalue' in data:
            (sti, lti, vlti) = ParseAttentionValue.parse(data)
            self.atomspace.set_av(h=Handle(id), sti=sti, lti=lti, vlti=vlti)

        atom = self.atomspace[Handle(id)]
        return {'atoms': marshal(atom, atom_fields)}

    def delete(self, id):
        """
        Removes an atom from the AtomSpace
        Uri: atoms/[id]

        :param id: Atom handle

        :return result:  Returns a JSON representation of the result, indicating success or failure.
        Example:

        {
          'result':
          {
            'handle': 2,
            'success': 'true'
          }
        }
        """

        if Handle(id) not in self.atomspace:
            abort(404, 'Handle not found')
        else:
            atom = self.atomspace[Handle(id)]

        status = self.atomspace.remove(atom)
        response = DeleteAtomResponse(id, status)
        return {'result': response.format()}
