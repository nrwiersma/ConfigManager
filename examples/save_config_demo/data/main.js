$( document ).ready(function() {

  var validationSettings = {
    rules: {
      device_name: {
        required: true
      },
      inching_delay: {
        required: true
      },
      led: {
        required: true
      }
    }
  };

  $.ajax({
         url: '/settings',
         success: function(data) {
           $.each(data, function(key, value, data) {
             var input = document.getElementsByName(key);
             if (input.length > 0) {
               var dataType = input[0].getAttribute("data-type");
               if (dataType == "boolean") {
                 $(input[0]).prop("checked", value);
                 return
               }

               $(input[0]).val(value);
             }
           });
         }
  });

  $.fn.serializeObject = function() {
    var o = {};
    
    var a = this.serializeArray();
    $.each(a, function() {
      var input = document.getElementsByName(this.name);
      var value = this.value;
      var dataType = input[0].getAttribute("data-type");
      if (dataType == "number") value = parseFloat(value);
      if (dataType == "boolean") value = value == "on";

      if (o[this.name]) {
        if (!o[this.name].push) {
          o[this.name] = [o[this.name]];
        }
        o[this.name].push(value || '');
      } else {
        o[this.name] = value || '';
      }
    });

    var c = $('input[type=radio],input[type=checkbox]',this);
    $.each(c, function(){
      if(!o.hasOwnProperty(this.name)){
        o[this.name] = false;
      }
    });

    return o;
  };

  $('form').validate(validationSettings);
  $('form').on('submit', function(e) {
    document.getElementById("status").innerHTML = "";
    if($(this).valid()) {
      e.preventDefault();

      var formData = $(this).serializeObject();

      // Send data as a PUT request
      $.ajax({
             url: '/settings',
             type: 'PUT',
             data: JSON.stringify(formData),
             contentType: 'application/json',
             success: function(data) {
               console.log(formData);
               document.getElementById("status").innerHTML = "Settings Updated"
             }
      });

      return false;
    }
  });
});
